#include "task_cmd_exec.h"

#include "FreeRTOS.h"
#include "driver_ad5664.h"
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
    // Control the 3 LEDs based on arguments 0-2
    // 0 = LED off, 1 = LED on
    if (pxCommand->args[0] == 0) {
        vLL_SetLED(LantanLED_Flt, LantanLED_Off);
    } else {
        vLL_SetLED(LantanLED_Flt, LantanLED_On);
    }
    
    if (pxCommand->args[1] == 0) {
        vLL_SetLED(LantanLED_Work, LantanLED_Off);
    } else {
        vLL_SetLED(LantanLED_Work, LantanLED_On);
    }
    
    if (pxCommand->args[2] == 0) {
        vLL_SetLED(LantanLED_Run, LantanLED_Off);
    } else {
        vLL_SetLED(LantanLED_Run, LantanLED_On);
    }
    
    // Send the 4th argument back through USB CDC
    vComm_Printf("INFO ACK:%lu\n", pxCommand->args[3]);
}

/**
 * @brief Handle VOLT_DC command
 *        Sets voltage for each of the 4 DAC channels (A-D)
 *        Arguments are in millivolts (0-2500 mV)
 * @param pxCommand Pointer to the command structure
 */
void vCmd_HandleVoltDC(const CommCommand_t *pxCommand) {
    // Set voltages for all 4 channels
    // args[0] = Channel A voltage in mV
    // args[1] = Channel B voltage in mV
    // args[2] = Channel C voltage in mV
    // args[3] = Channel D voltage in mV
    AD5664_SetAllVoltages(pxCommand->args[0], pxCommand->args[1], 
                          pxCommand->args[2], pxCommand->args[3]);
    
    // Send acknowledgment
    vComm_Printf("VOLT_DC ACK:%lu;%lu;%lu;%lu\n", 
                 pxCommand->args[0], pxCommand->args[1], 
                 pxCommand->args[2], pxCommand->args[3]);
}

/**
 * @brief Main command execution task
 *        Sends HELLO message 500ms after boot, then processes incoming commands
 * @param pvParams Task parameters (unused)
 */
void vCmd_MainTask(void *pvParams) {
    UNUSED(pvParams);
    
    // Send HELLO message 500ms after boot
    vTaskDelay(pdMS_TO_TICKS(500));
    vComm_Printf("HELLO\r\n");

    AD5664_Init();
    
    CommCommand_t xCommand;
    
    while (1) {
        // Wait for a command from the queue (blocking)
        // if (xComm_ReceiveCommand(&xCommand, pdMS_TO_TICKS(500)) == pdTRUE) {
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

        AD5664_SetAllVoltages(500, 500, 500, 500);
        // AD5664_SetVoltage(AD5664_CHANNEL_A, 500);
        vTaskDelay(pdMS_TO_TICKS(2000));
        AD5664_SetAllVoltages(1500, 1500, 1500, 1500);
        // AD5664_SetVoltage(AD5664_CHANNEL_A, 1500);
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}
