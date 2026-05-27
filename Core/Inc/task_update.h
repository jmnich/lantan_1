#ifndef TASK_UPDATE_H_
#define TASK_UPDATE_H_

#include "lantan_ll.h"
#include "task_comm.h"
#include "driver_ad5664.h"

// Global variables for UPDATE message fields (accessed by sendUpdate)
extern volatile uint8_t g_Update_PowerGoodFlag;
extern volatile uint8_t g_Update_ChannelA_Active;
extern volatile uint8_t g_Update_ChannelB_Active;
extern volatile uint8_t g_Update_ChannelC_Active;
extern volatile uint8_t g_Update_ChannelD_Active;
extern volatile uint32_t g_Update_DutVoltageA_uV;
extern volatile uint32_t g_Update_DutVoltageB_uV;
extern volatile uint32_t g_Update_DutVoltageC_uV;
extern volatile uint32_t g_Update_DutVoltageD_uV;
extern volatile uint32_t g_Update_DutCurrentA_uA;
extern volatile uint32_t g_Update_DutCurrentB_uA;
extern volatile uint32_t g_Update_DutCurrentC_uA;
extern volatile uint32_t g_Update_DutCurrentD_uA;
extern volatile uint32_t g_Update_DutModAmplitudeA_uA;
extern volatile uint32_t g_Update_DutModAmplitudeB_uA;
extern volatile uint32_t g_Update_DutModAmplitudeC_uA;
extern volatile uint32_t g_Update_DutModAmplitudeD_uA;
extern volatile uint32_t g_Update_DutResponseA;
extern volatile uint32_t g_Update_DutResponseB;
extern volatile uint32_t g_Update_DutResponseC;
extern volatile uint32_t g_Update_DutResponseD;
extern volatile uint32_t g_Update_DetectorSensitivity;
extern volatile uint32_t g_Update_DetectorGain;

void vUpdate_MainTask(void *pvParams);

#endif // TASK_UPDATE_H_
