#ifndef TASK_COMM_H_
#define TASK_COMM_H_

#include "stm32h7xx_hal.h"
#include "lantan_ll.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"

// Command IDs enum
typedef enum {
    COMM_CMD_INVALID = 0,
    COMM_CMD_INFO,
    COMM_CMD_VOLT_DC,
    COMM_CMD_MODULATOR,
    COMM_CMD_DETECTOR,
    COMM_CMD_MAX
} CommCommandID_t;

// Command structure with ID and 4 unsigned integer arguments
typedef struct {
    CommCommandID_t id;
    uint32_t args[32];
} CommCommand_t;

// Queue handle for incoming commands
extern QueueHandle_t xCommCommandQueue;

// Buffer sizes
#define COMM_RX_BUFFER_SIZE    256
#define COMM_TX_BUFFER_SIZE    256
#define COMM_QUEUE_SIZE        16

// Function prototypes
void vComm_MainTask(void *pvParams);
void vComm_Init(void);
BaseType_t xComm_SendCommand(CommCommand_t *pxCommand);
BaseType_t xComm_ReceiveCommand(CommCommand_t *pxCommand, TickType_t xTicksToWait);
void vComm_ProcessRxData(void);
int vComm_Printf(const char *pcFormat, ...);

#endif // TASK_COMM_H_
