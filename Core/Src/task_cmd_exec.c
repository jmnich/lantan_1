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
    vComm_Printf("VOLT_DC ACK:%lu;%lu;%lu;%lu\n", 
                 pxCommand->args[0], pxCommand->args[1],
                 pxCommand->args[2], pxCommand->args[3]);
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

    vComm_Printf("HELLO\r\n");

    // AD5664_Init();
    // pull CS high
    HAL_GPIO_WritePin(SPI4_CS_GPIO_Port, SPI4_CS_Pin, GPIO_PIN_SET);

    vLL_Set9VRail(1);  

    vLL_DetectorConfigure(DetRange_100k, DetGain_1_0);

    vSynth_CalculateChannel(SynthChannel_A, 446, 112);
    // vSynth_CalculateChannel(SynthChannel_A, 50, 10);
    vSynth_CalculateChannel(SynthChannel_B, 1000, 500);
    vSynth_CalculateChannel(SynthChannel_C, 1000, 500);
    vSynth_CalculateChannel(SynthChannel_D, 1000, 500);

    vSynth_SetChannelEnabled(SynthChannel_A, 1);
    // vSynth_SetChannelEnabled(SynthChannel_B, 1);
    // vSynth_SetChannelEnabled(SynthChannel_C, 1);
    // vSynth_SetChannelEnabled(SynthChannel_D, 1);
    uSynth_StartSynth();

    // float demodValA = fDemod_SingleFreq(synthFrequency[0], DemodSrc_Diagnostic, AD5664_CHANNEL_A);    
    // float demodValB = fDemod_SingleFreq(synthFrequency[1], DemodSrc_Diagnostic, AD5664_CHANNEL_B);    
    // float demodValC = fDemod_SingleFreq(synthFrequency[2], DemodSrc_Diagnostic, AD5664_CHANNEL_C);    
    // float demodValD = fDemod_SingleFreq(synthFrequency[3], DemodSrc_Diagnostic, AD5664_CHANNEL_D);    

    // AD5664_SetVoltage(AD5664_CHANNEL_A, 1500);
    // AD5664_SetVoltage(AD5664_CHANNEL_B, 500);
    // AD5664_SetVoltage(AD5664_CHANNEL_C, 500);
    // AD5664_SetVoltage(AD5664_CHANNEL_D, 500);

    vLL_CurrentSourceVoltage(LantanCurrSrc_A, LantanSrcVolt5V);
    vLL_CurrentSourceVoltage(LantanCurrSrc_B, LantanSrcVolt5V);
    vLL_CurrentSourceVoltage(LantanCurrSrc_C, LantanSrcVolt5V);
    vLL_CurrentSourceVoltage(LantanCurrSrc_D, LantanSrcVolt5V);
  
    vLL_CurrentSourceRelease(LantanCurrSrc_A, LantanSrcReleased);

    CommCommand_t xCommand;
    
    while (1) {
        // vTaskDelay(pdMS_TO_TICKS(50));
        float demodValA = fDemod_SingleFreq(synthFrequency[0], DemodSrc_Detector, AD5664_CHANNEL_A);    

        // Print demodValA to output with 5 significant digits
        int32_t demodInt = (int32_t)(demodValA * 100000);
        vComm_Printf("DEMOD_A:%ld\n", demodInt);

        // Don' delete comments below!
        // // Wait for a command from the queue (blocking)
        // if (xComm_ReceiveCommand(&xCommand, portMAX_DELAY) == pdTRUE) {
        //     // Dispatch command to appropriate handler
        //     switch (xCommand.id) {
        //         case COMM_CMD_INFO:
        //             vCmd_HandleInfo(&xCommand);
        //             break;
                    
        //         case COMM_CMD_VOLT_DC:
        //             vCmd_HandleVoltDC(&xCommand);
        //             break;
                    
        //         case COMM_CMD_INVALID:
        //         default:
        //             // Invalid command - send error
        //             vComm_Printf("ERROR:Invalid command\n");
        //             break;
        //     }
        // }
    }
}
