#include "task_comm.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

void vComm_MainTask(void * pvParams) {

    while(1) {            
        vLL_SetLED(LantanLED_Flt, LantanLED_On);
        HAL_Delay(1000);
        vLL_SetLED(LantanLED_Work, LantanLED_On);
        HAL_Delay(1000);
        vLL_SetLED(LantanLED_Run, LantanLED_On);
        HAL_Delay(1000);
        
        vLL_SetLED(LantanLED_Flt, LantanLED_Off);
        HAL_Delay(1000);
        vLL_SetLED(LantanLED_Work, LantanLED_Off);
        HAL_Delay(1000);
        vLL_SetLED(LantanLED_Run, LantanLED_Off);
        HAL_Delay(1000);

    }
}
