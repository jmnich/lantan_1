#include "task_comm.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "usbd_cdc_if.h"
#include "usb_device.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

// Queue handle for incoming commands
QueueHandle_t xCommCommandQueue = NULL;

// RX buffer and state
static uint8_t ucCommRxBuffer[COMM_RX_BUFFER_SIZE];
static uint32_t ulCommRxPos = 0;

// TX buffer
static uint8_t ucCommTxBuffer[COMM_TX_BUFFER_SIZE];

// Semaphore for TX synchronization
static SemaphoreHandle_t xCommTxSemaphore = NULL;

// Command string mapping for debugging
static const char *pcCommCommandNames[COMM_CMD_MAX] = {
    "INVALID",
    "INFO",
    "VOLT_DC"
};

// String to command ID mapping
static const char *pcCommCommandStrings[COMM_CMD_MAX] = {
    "",
    "INFO",
    "VOLT_DC"
};

// Forward declarations
static CommCommandID_t eComm_ParseCommandID(const char *pcCmdStr);
static BaseType_t xComm_ParseCommand(const char *pcBuffer, CommCommand_t *pxCommand);

/**
 * @brief Initialize communication task resources
 */
void vComm_Init(void) {
    // Create command queue
    xCommCommandQueue = xQueueCreate(COMM_QUEUE_SIZE, sizeof(CommCommand_t));
    configASSERT(xCommCommandQueue != NULL);

    // Create TX semaphore
    xCommTxSemaphore = xSemaphoreCreateBinary();
    configASSERT(xCommTxSemaphore != NULL);
    xSemaphoreGive(xCommTxSemaphore); // Initially available
}

/**
 * @brief Main communication task
 * @param pvParams Task parameters (unused)
 */
void vComm_MainTask(void *pvParams) {
    UNUSED(pvParams);
    
    // Initialize communication
    vComm_Init();

    // Initialize USB device
    MX_USB_DEVICE_Init();

    // Main loop
    while (1) {
        // Process any received data
        vComm_ProcessRxData();

        // Small delay to prevent CPU hogging
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}

/**
 * @brief Process received USB data
 *        This function is called from the main task and from the USB RX callback
 */
void vComm_ProcessRxData(void) {
    // Check if we have data to process
    if (ulCommRxPos > 0) {
        // Look for newline character
        for (uint32_t i = 0; i < ulCommRxPos; i++) {
            if (ucCommRxBuffer[i] == '\n') {
                // Found a complete message
                CommCommand_t xCommand;
                
                // Null-terminate the string at the newline
                ucCommRxBuffer[i] = '\0';
                
                // Parse the command
                if (xComm_ParseCommand((const char *)ucCommRxBuffer, &xCommand) == pdTRUE) {
                    // Send to queue
                    xComm_SendCommand(&xCommand);
                }
                
                // Move remaining data to the beginning
                uint32_t ulRemaining = ulCommRxPos - i - 1;
                if (ulRemaining > 0) {
                    memmove(ucCommRxBuffer, &ucCommRxBuffer[i + 1], ulRemaining);
                }
                ulCommRxPos = ulRemaining;
                
                // Continue processing in case there are more messages
                continue;
            }
        }
        
        // Check if buffer is full (no newline found but buffer is full)
        if (ulCommRxPos >= COMM_RX_BUFFER_SIZE - 1) {
            // Buffer overflow - discard data
            ulCommRxPos = 0;
        }
    }
}

/**
 * @brief USB RX callback - called by CDC_Receive_FS when data is received
 * @param Buf Received data buffer
 * @param Len Number of bytes received
 */
void USBD_CDC_RxCallback(uint8_t *Buf, uint32_t *Len) {
    (void)Len; // Unused
    
    // Copy received data to our buffer
    for (uint32_t i = 0; i < *Len; i++) {
        if (ulCommRxPos < COMM_RX_BUFFER_SIZE - 1) {
            ucCommRxBuffer[ulCommRxPos++] = Buf[i];
        }
    }
}

/**
 * @brief Send a command to the command queue
 * @param pxCommand Pointer to the command to send
 * @return pdTRUE if the command was queued, pdFALSE otherwise
 */
BaseType_t xComm_SendCommand(CommCommand_t *pxCommand) {
    if (xCommCommandQueue == NULL) {
        return pdFALSE;
    }
    
    BaseType_t xResult = xQueueSendToBack(xCommCommandQueue, pxCommand, pdMS_TO_TICKS(100));
    return (xResult == pdPASS) ? pdTRUE : pdFALSE;
}

/**
 * @brief Receive a command from the command queue
 * @param pxCommand Pointer to the command to receive
 * @param xTicksToWait Maximum time to wait for a command
 * @return pdTRUE if a command was received, pdFALSE otherwise
 */
BaseType_t xComm_ReceiveCommand(CommCommand_t *pxCommand, TickType_t xTicksToWait) {
    if (xCommCommandQueue == NULL) {
        return pdFALSE;
    }
    
    BaseType_t xResult = xQueueReceive(xCommCommandQueue, pxCommand, xTicksToWait);
    return (xResult == pdPASS) ? pdTRUE : pdFALSE;
}

/**
 * @brief Parse command ID from string
 * @param pcCmdStr Command string
 * @return Command ID enum value
 */
static CommCommandID_t eComm_ParseCommandID(const char *pcCmdStr) {
    for (uint32_t i = 1; i < COMM_CMD_MAX; i++) {
        if (strcmp(pcCmdStr, pcCommCommandStrings[i]) == 0) {
            return (CommCommandID_t)i;
        }
    }
    return COMM_CMD_INVALID;
}

/**
 * @brief Parse a complete command string
 * @param pcBuffer Command string buffer (format: "CMD;arg1;arg2;arg3;arg4\n")
 * @param pxCommand Pointer to command struct to populate
 * @return pdTRUE if parsing succeeded, pdFALSE otherwise
 */
static BaseType_t xComm_ParseCommand(const char *pcBuffer, CommCommand_t *pxCommand) {
    if (pcBuffer == NULL || pxCommand == NULL) {
        return pdFALSE;
    }
    
    // Make a copy of the buffer on the stack for parsing
    char acBufferCopy[COMM_RX_BUFFER_SIZE];
    strncpy(acBufferCopy, pcBuffer, sizeof(acBufferCopy) - 1);
    acBufferCopy[sizeof(acBufferCopy) - 1] = '\0';
    
    // Split the string by semicolons
    char *pcToken = strtok(acBufferCopy, ";");
    
    // First token should be the command ID
    if (pcToken == NULL) {
        return pdFALSE;
    }
    
    CommCommandID_t eCmdID = eComm_ParseCommandID(pcToken);
    if (eCmdID == COMM_CMD_INVALID) {
        return pdFALSE;
    }
    
    pxCommand->id = eCmdID;
    
    // Parse the remaining 4 arguments
    for (uint32_t i = 0; i < 4; i++) {
        pcToken = strtok(NULL, ";\n\r");
        if (pcToken == NULL) {
            // Missing arguments - default to 0
            pxCommand->args[i] = 0;
            continue;
        }
        
        // Convert to unsigned long
        char *pcEnd;
        unsigned long ulValue = strtoul(pcToken, &pcEnd, 10);
        
        if (pcEnd == pcToken) {
            // Conversion failed - default to 0
            pxCommand->args[i] = 0;
        } else {
            pxCommand->args[i] = (uint32_t)ulValue;
        }
    }
    
    return pdTRUE;
}

/**
 * @brief Thread-safe printf-like function for USB CDC output
 * @param pcFormat Format string
 * @param ... Variable arguments
 * @return Number of bytes transmitted, or negative on error
 */
int vComm_Printf(const char *pcFormat, ...) {
    if (xCommTxSemaphore == NULL) {
        return -1;
    }
    
    // Wait for TX semaphore
    if (xSemaphoreTake(xCommTxSemaphore, pdMS_TO_TICKS(1000)) != pdTRUE) {
        return -1;
    }
    
    // Format the string
    va_list xArgs;
    va_start(xArgs, pcFormat);
    int iLen = vsnprintf((char *)ucCommTxBuffer, COMM_TX_BUFFER_SIZE, pcFormat, xArgs);
    va_end(xArgs);
    
    if (iLen <= 0) {
        xSemaphoreGive(xCommTxSemaphore);
        return iLen;
    }
    
    // Ensure null termination
    if (iLen >= COMM_TX_BUFFER_SIZE) {
        iLen = COMM_TX_BUFFER_SIZE - 1;
        ucCommTxBuffer[iLen] = '\0';
    }
    
    // Transmit over USB CDC
    uint8_t ucResult = CDC_Transmit_FS(ucCommTxBuffer, (uint16_t)iLen);
    
    // Release semaphore
    xSemaphoreGive(xCommTxSemaphore);
    
    if (ucResult == USBD_OK) {
        return iLen;
    } else {
        return -1;
    }
}
