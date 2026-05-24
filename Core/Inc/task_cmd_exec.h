#ifndef TASK_CMD_EXEC_H_
#define TASK_CMD_EXEC_H_

#include "lantan_ll.h"
#include "task_comm.h"

// Function prototypes
void vCmd_MainTask(void *pvParams);

// Command handler functions
void vCmd_HandleInfo(const CommCommand_t *pxCommand);
void vCmd_HandleVoltDC(const CommCommand_t *pxCommand);

#endif // TASK_CMD_EXEC_H_
