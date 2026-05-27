#ifndef TASK_UPDATE_H_
#define TASK_UPDATE_H_

#include "lantan_ll.h"
#include "task_comm.h"
#include "driver_ad5664.h"

extern uint32_t powerGoodFlag;

void vUpdate_MainTask(void *pvParams);

#endif // TASK_UPDATE_H_
