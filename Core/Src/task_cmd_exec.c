#include "task_cmd_exec.h"

#include "FreeRTOS.h"
#include "driver_ad5664.h"
#include "lantan_ll.h"
#include "lantan_synth.h"
#include "lantan_demodulator.h"

#include "portmacro.h"
#include "projdefs.h"
#include "task.h"
#include "queue.h"

/**
 * @brief Handle INFO command
 *        Controls the 3 LEDs based on first 3 arguments (0=off, 1=on)
 *        Sends the 4th argument back through USB CDC
 * @param pxCommand Pointer to the command structure
 */
void vCmd_HandleInfo(const CommCommand_t *pxCommand) {    
    vComm_Printf("INFO ACK\n");
}

/**
 * @brief Handle VOLT_DC command
 *        Sets voltage for each of the 4 DAC channels (A-D)
 *        Arguments are in millivolts (0-2500 mV)
 * @param pxCommand Pointer to the command structure
 */
void vCmd_HandleVoltDC(const CommCommand_t *pxCommand) {
    // Set voltages for all 4 channels
    AD5664_SetVoltage(AD5664_CHANNEL_A, pxCommand->args[0]);
    AD5664_SetVoltage(AD5664_CHANNEL_B, pxCommand->args[1]);
    AD5664_SetVoltage(AD5664_CHANNEL_C, pxCommand->args[2]);
    AD5664_SetVoltage(AD5664_CHANNEL_D, pxCommand->args[3]);
    
    // Send acknowledgment
    vComm_Printf("VOLT_DC ACK:%lu|%lu|%lu|%lu\r\n", 
                 pxCommand->args[0], pxCommand->args[1],
                 pxCommand->args[2], pxCommand->args[3]);
}

/**
 * @brief Handle MODULATOR command
 *        Configures modulator channels and amplitudes
 *        Args: channel A active (0/1)|channel B active (0/1)|channel C active (0/1)|channel D active (0/1)|
 *              mod amplitude A (%)|mod amplitude B (%)|mod amplitude C (%)|mod amplitude D (%)
 * @param pxCommand Pointer to the command structure
 */
void vCmd_HandleModulator(const CommCommand_t *pxCommand) {
    // pxCommand->args[0-3]: Channel A-D active flags (0=off, 1=on)
    // pxCommand->args[4-7]: Modulation amplitude A-D in % (0-100)
    
    vSynth_CalculateChannel(SynthChannel_A, 446, 112);
    // vSynth_CalculateChannel(SynthChannel_A, 50, 10);
    vSynth_CalculateChannel(SynthChannel_B, 1000, 500);
    vSynth_CalculateChannel(SynthChannel_C, 1000, 500);
    vSynth_CalculateChannel(SynthChannel_D, 1000, 500);

    if(pxCommand->args[4] == 0) { 
        vSynth_SetChannelEnabled(SynthChannel_A, 0);
        vLL_CurrentSourceRelease(LantanCurrSrc_A, LantanSrcLocked);
    }
    else {
        vSynth_SetChannelEnabled(SynthChannel_A, 1);
        vLL_CurrentSourceRelease(LantanCurrSrc_A, LantanSrcReleased);
    }

    if(pxCommand->args[5] == 0) { 
        vSynth_SetChannelEnabled(SynthChannel_B, 0);
        vLL_CurrentSourceRelease(LantanCurrSrc_B, LantanSrcLocked);
    }
    else {
        vSynth_SetChannelEnabled(SynthChannel_B, 1);
        vLL_CurrentSourceRelease(LantanCurrSrc_B, LantanSrcReleased);
    }

    if(pxCommand->args[6] == 0) { 
        vSynth_SetChannelEnabled(SynthChannel_C, 0);
        vLL_CurrentSourceRelease(LantanCurrSrc_C, LantanSrcLocked);
    }
    else {
        vSynth_SetChannelEnabled(SynthChannel_C, 1);
        vLL_CurrentSourceRelease(LantanCurrSrc_C, LantanSrcReleased);
    }

    if(pxCommand->args[7] == 0) { 
        vSynth_SetChannelEnabled(SynthChannel_D, 0);
        vLL_CurrentSourceRelease(LantanCurrSrc_D, LantanSrcLocked);
    }
    else {
        vSynth_SetChannelEnabled(SynthChannel_D, 1);
        vLL_CurrentSourceRelease(LantanCurrSrc_D, LantanSrcReleased);
    }

    uSynth_StartSynth();
}

/**
 * @brief Handle DETECTOR command
 *        Configures detector sensitivity and gain
 *        Args: detector sensitivity (1-4)|detector gain (1-4)
 * @param pxCommand Pointer to the command structure
 */
void vCmd_HandleDetector(const CommCommand_t *pxCommand) {
    // pxCommand->args[0]: Detector sensitivity (1, 2, 3, or 4)
    // pxCommand->args[1]: Detector gain (1, 2, 3, or 4)
    
    vLL_DetectorConfigure((LantanDetectorRange_t)pxCommand->args[0], (LantanDetectorGain_t)pxCommand->args[1]);
}

/**
 * @brief Main command execution task
 * @param pvParams Task parameters (unused)
 */
void vCmd_MainTask(void *pvParams) {
    UNUSED(pvParams);
    
    // Send HELLO message 500ms after boot
    vLL_SetLED(LantanLED_Flt, LantanLED_On);
    vLL_SetLED(LantanLED_Work, LantanLED_On);
    vLL_SetLED(LantanLED_Run, LantanLED_On);
    vTaskDelay(pdMS_TO_TICKS(500));

    vLL_SetLED(LantanLED_Flt, LantanLED_Off);
    vLL_SetLED(LantanLED_Work, LantanLED_Off);

    // vComm_Printf("HELLO\r\n");

    // AD5664_Init();
    // pull CS high
    HAL_GPIO_WritePin(SPI4_CS_GPIO_Port, SPI4_CS_Pin, GPIO_PIN_SET);

    vLL_Set9VRail(1);  
    vLL_DetectorConfigure(DetRange_100k, DetGain_1_0);
    vLL_CurrentSourceRelease(LantanCurrSrc_A, LantanSrcLocked);
    vLL_CurrentSourceRelease(LantanCurrSrc_B, LantanSrcLocked);
    vLL_CurrentSourceRelease(LantanCurrSrc_C, LantanSrcLocked);
    vLL_CurrentSourceRelease(LantanCurrSrc_D, LantanSrcLocked);

    CommCommand_t xCommand;
    
    while (1) {
        // Wait for a command from the queue (blocking)
        if (xComm_ReceiveCommand(&xCommand, portMAX_DELAY) == pdTRUE) {
            // Dispatch command to appropriate handler
            switch (xCommand.id) {
                case COMM_CMD_INFO:
                    vCmd_HandleInfo(&xCommand);
                    break;
                    
                case COMM_CMD_VOLT_DC:
                    vCmd_HandleVoltDC(&xCommand);
                    break;
                
                case COMM_CMD_MODULATOR:
                    vCmd_HandleModulator(&xCommand);
                    break;
                
                case COMM_CMD_DETECTOR:
                    vCmd_HandleDetector(&xCommand);
                    break;
                    
                case COMM_CMD_INVALID:
                default:
                    // Invalid command - send error
                    vComm_Printf("ERROR:Invalid command\r\n");
                    break;
            }
        }
    }
}
