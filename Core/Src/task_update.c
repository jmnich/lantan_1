#include "task_update.h"
#include "lantan_synth.h"
#include "lantan_demodulator.h"
#include "task_cmd_exec.h"
#include <math.h>

// Static global variables for UPDATE message fields
static volatile uint8_t g_Update_PowerGoodFlag = 0;
static volatile uint8_t g_Update_ChannelA_Active = 0;
static volatile uint8_t g_Update_ChannelB_Active = 0;
static volatile uint8_t g_Update_ChannelC_Active = 0;
static volatile uint8_t g_Update_ChannelD_Active = 0;
static volatile uint32_t g_Update_DutVoltageA_uV = 0;
static volatile uint32_t g_Update_DutVoltageB_uV = 0;
static volatile uint32_t g_Update_DutVoltageC_uV = 0;
static volatile uint32_t g_Update_DutVoltageD_uV = 0;
static volatile uint32_t g_Update_DutCurrentA_uA = 0;
static volatile uint32_t g_Update_DutCurrentB_uA = 0;
static volatile uint32_t g_Update_DutCurrentC_uA = 0;
static volatile uint32_t g_Update_DutCurrentD_uA = 0;
static volatile uint32_t g_Update_DutModAmplitudeA_uA = 0;
static volatile uint32_t g_Update_DutModAmplitudeB_uA = 0;
static volatile uint32_t g_Update_DutModAmplitudeC_uA = 0;
static volatile uint32_t g_Update_DutModAmplitudeD_uA = 0;
static volatile uint32_t g_Update_DutResponseA = 0;
static volatile uint32_t g_Update_DutResponseB = 0;
static volatile uint32_t g_Update_DutResponseC = 0;
static volatile uint32_t g_Update_DutResponseD = 0;
static volatile uint32_t g_Update_DetectorSensitivity = 1;
static volatile uint32_t g_Update_DetectorGain = 1;

static void sendUpdate(void) {
    // Format and send update message using global variables from this file
    // Protocol format: UPDATE|<power good flag>|<channel A active>|...|<detector gain>\r\n
    vComm_Printf("UPDATE|%u|%u|%u|%u|%u|%lu|%lu|%lu|%lu|%lu|%lu|%lu|%lu|%lu|%lu|%lu|%lu|%lu|%lu|%lu|%lu|%lu|%lu\r\n",
                 g_Update_PowerGoodFlag,
                 g_Update_ChannelA_Active,
                 g_Update_ChannelB_Active,
                 g_Update_ChannelC_Active,
                 g_Update_ChannelD_Active,
                 g_Update_DutVoltageA_uV,
                 g_Update_DutVoltageB_uV,
                 g_Update_DutVoltageC_uV,
                 g_Update_DutVoltageD_uV,
                 g_Update_DutCurrentA_uA,
                 g_Update_DutCurrentB_uA,
                 g_Update_DutCurrentC_uA,
                 g_Update_DutCurrentD_uA,
                 g_Update_DutModAmplitudeA_uA,
                 g_Update_DutModAmplitudeB_uA,
                 g_Update_DutModAmplitudeC_uA,
                 g_Update_DutModAmplitudeD_uA,
                 g_Update_DutResponseA,
                 g_Update_DutResponseB,
                 g_Update_DutResponseC,
                 g_Update_DutResponseD,
                 g_Update_DetectorSensitivity,
                 g_Update_DetectorGain);
}

void vUpdate_MainTask(void *pvParams) {
    UNUSED(pvParams);

    while(1) {
        // repeat this loop as often as possible
        // Update the global variables above with current values from hardware
        g_Update_PowerGoodFlag = 1;
        g_Update_ChannelA_Active = synthChannelActive[0];
        g_Update_ChannelB_Active = synthChannelActive[1];
        g_Update_ChannelC_Active = synthChannelActive[2];
        g_Update_ChannelD_Active = synthChannelActive[3];
        g_Update_DutCurrentA_uA = (uint32_t)((float)synthOffsets[0] * 0.45 * 1000.0);
        g_Update_DutCurrentB_uA = (uint32_t)((float)synthOffsets[1] * 0.45 * 1000.0);
        g_Update_DutCurrentC_uA = (uint32_t)((float)synthOffsets[2] * 0.45 * 1000.0);
        g_Update_DutCurrentD_uA = (uint32_t)((float)synthOffsets[3] * 0.45 * 1000.0);
        g_Update_DutModAmplitudeA_uA = (uint32_t)(((float)synthPkPkMax[0] * (float)modulationAmpsSetByUser[0] / 100.0) * 0.45 * 1000.0);
        g_Update_DutModAmplitudeB_uA = (uint32_t)(((float)synthPkPkMax[1] * (float)modulationAmpsSetByUser[1] / 100.0) * 0.45 * 1000.0);
        g_Update_DutModAmplitudeC_uA = (uint32_t)(((float)synthPkPkMax[2] * (float)modulationAmpsSetByUser[2] / 100.0) * 0.45 * 1000.0);
        g_Update_DutModAmplitudeD_uA = (uint32_t)(((float)synthPkPkMax[3] * (float)modulationAmpsSetByUser[3] / 100.0) * 0.45 * 1000.0);
        g_Update_DetectorSensitivity = currentDetectorRange + 1;
        g_Update_DetectorGain = currentDetectorGain + 1;        
        
        uint32_t response[4] = {0};
        
        if(synthChannelActive[0]) {
            response[0] = (uint32_t)(roundf(fDemod_SingleFreq(synthFrequency[0], DemodSrc_Detector, AD5664_CHANNEL_A) * 1E6));
        }

        if(synthChannelActive[1]) {
            response[1] = (uint32_t)(roundf(fDemod_SingleFreq(synthFrequency[1], DemodSrc_Detector, AD5664_CHANNEL_A) * 1E6));
        }

        if(synthChannelActive[2]) {
            response[2] = (uint32_t)(roundf(fDemod_SingleFreq(synthFrequency[2], DemodSrc_Detector, AD5664_CHANNEL_A) * 1E6));
        }

        if(synthChannelActive[3]) {
            response[3] = (uint32_t)(roundf(fDemod_SingleFreq(synthFrequency[3], DemodSrc_Detector, AD5664_CHANNEL_A) * 1E6));
        }

        g_Update_DutResponseA = response[0];
        g_Update_DutResponseB = response[1];
        g_Update_DutResponseC = response[2];
        g_Update_DutResponseD = response[3];

        // TODO 
        // measure and filter voltage for SRC A-D
        // SRC_V_A - 
        // SRC_V_B - 
        // SRC_V_C -
        // SRC_V_D - 

        g_Update_DutVoltageA_uV = 1E6;
        g_Update_DutVoltageB_uV = 1E6;
        g_Update_DutVoltageC_uV = 1E6;
        g_Update_DutVoltageD_uV = 1E6;

        sendUpdate();
        
        if((synthChannelActive[0] == 0) && 
            (synthChannelActive[1] == 0) && 
            (synthChannelActive[2] == 0) &&     
            (synthChannelActive[3] == 0)) {

            // Small delay to prevent CPU hogging
            vTaskDelay(pdMS_TO_TICKS(150));
        }
    }
}