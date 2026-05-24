#include "task_cmd_exec.h"

#include "FreeRTOS.h"
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
 *        Placeholder for future implementation
 * @param pxCommand Pointer to the command structure
 */
void vCmd_HandleVoltDC(const CommCommand_t *pxCommand) {
    // Placeholder for VOLT_DC command handling
    // For now, just send an acknowledgment
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
    
    CommCommand_t xCommand;
    
    while (1) {
        // Wait for a command from the queue (blocking)
        if (xComm_ReceiveCommand(&xCommand, pdMS_TO_TICKS(500)) == pdTRUE) {
            // Dispatch command to appropriate handler
            switch (xCommand.id) {
                case COMM_CMD_INFO:
                    vCmd_HandleInfo(&xCommand);
                    break;
                    
                case COMM_CMD_VOLT_DC:
                    vCmd_HandleVoltDC(&xCommand);
                    break;
                    
                case COMM_CMD_INVALID:
                default:
                    // Invalid command - send error
                    vComm_Printf("ERROR:Invalid command\n");
                    break;
            }
        }

        // vTaskDelay(pdMS_TO_TICKS(500));
        vComm_Printf("HELLO\r\n");
    }
}
