#include "task_update.h"
#include "lantan_synth.h"
#include "lantan_demodulator.h"
#include "task_cmd_exec.h"
#include "lantan_ll.h"
#include "adc.h"
#include <math.h>

uint32_t powerGoodFlag = 0;

// Helper to read ADC3 channel and convert to millivolts
static uint32_t readADC3Channel_mV(uint32_t channel) {
    ADC_ChannelConfTypeDef sConfig = {0};
    uint32_t adcValue;
    
    sConfig.Channel = channel;
    sConfig.Rank = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
    sConfig.SingleDiff = ADC_SINGLE_ENDED;
    sConfig.OffsetNumber = ADC_OFFSET_NONE;
    sConfig.Offset = 0;
    sConfig.OffsetSignedSaturation = DISABLE;
    
    if (HAL_ADC_ConfigChannel(&hadc3, &sConfig) != HAL_OK) {
        return 0;
    }
    
    if (HAL_ADC_Start(&hadc3) != HAL_OK) {
        return 0;
    }
    
    if (HAL_ADC_PollForConversion(&hadc3, 10) != HAL_OK) {
        HAL_ADC_Stop(&hadc3);
        return 0;
    }
    
    adcValue = HAL_ADC_GetValue(&hadc3);
    HAL_ADC_Stop(&hadc3);
    
    // Convert to millivolts: (adcValue * Vref_mV) / 65535
    return (uint32_t)((adcValue * LANTAN_ADC_VREF_mV) / 65535.0f);
}

// Static global variables for UPDATE message fields
static volatile uint8_t g_Update_PowerGoodFlag = 0;

// Voltage measurement: circular buffers for 10-sample moving average
#define VOLTAGE_FILTER_SAMPLES 10
static uint32_t voltageSamples[4][VOLTAGE_FILTER_SAMPLES] = {0};
static uint8_t voltageSampleIndex[4] = {0};
static uint8_t voltageSampleCount[4] = {0};
static uint32_t voltageSum[4] = {0};

// Helper to read ADC2 channel and convert to microvolts
static uint32_t readADC2Channel(uint32_t channel) {
    ADC_ChannelConfTypeDef sConfig = {0};
    uint32_t adcValue;
    
    sConfig.Channel = channel;
    sConfig.Rank = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
    sConfig.SingleDiff = ADC_SINGLE_ENDED;
    sConfig.OffsetNumber = ADC_OFFSET_NONE;
    sConfig.Offset = 0;
    sConfig.OffsetSignedSaturation = DISABLE;
    
    if (HAL_ADC_ConfigChannel(&hadc2, &sConfig) != HAL_OK) {
        return 0;
    }
    
    if (HAL_ADC_Start(&hadc2) != HAL_OK) {
        return 0;
    }
    
    if (HAL_ADC_PollForConversion(&hadc2, 10) != HAL_OK) {
        HAL_ADC_Stop(&hadc2);
        return 0;
    }
    
    adcValue = HAL_ADC_GetValue(&hadc2);
    HAL_ADC_Stop(&hadc2);
    
    // Convert to voltage in mV: (adcValue * Vref_mV) / 65535
    // Then apply correction: u * 2 - 240 mV
    // Return in microvolts
    float voltage_mV = (adcValue * LANTAN_ADC_VREF_mV) / 65535.0f;
    float corrected_mV = voltage_mV * 2.0f - 240.0f;
    return (uint32_t)(corrected_mV * 1000.0f);
}

// Static global variables for UPDATE message fields
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

        // Power good flag: 1 if voltage at ADC3 IN0 >= 0.8V, otherwise 0
        uint32_t voltage_mV = readADC3Channel_mV(ADC_CHANNEL_0);
        powerGoodFlag = (voltage_mV >= 1000) ? 1 : 0;

        // just a little safety to avoid blowing up the USB
        if(powerGoodFlag != 1) {
            vLL_CurrentSourceRelease(LantanCurrSrc_A, LantanSrcLocked);
            vLL_CurrentSourceRelease(LantanCurrSrc_B, LantanSrcLocked);
            vLL_CurrentSourceRelease(LantanCurrSrc_C, LantanSrcLocked);
            vLL_CurrentSourceRelease(LantanCurrSrc_D, LantanSrcLocked);
        }         

        g_Update_PowerGoodFlag = powerGoodFlag;
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

        // Measure and filter voltage for SRC A-D
        // SRC_V_A - ADC2 IN7, SRC_V_B - ADC2 IN4, SRC_V_C - ADC2 IN8, SRC_V_D - ADC2 IN9
        static const uint32_t adcChannels[4] = {
            ADC_CHANNEL_7,  // A
            ADC_CHANNEL_4,  // B
            ADC_CHANNEL_8,  // C
            ADC_CHANNEL_9   // D
        };
        
        for (int ch = 0; ch < 4; ch++) {
            uint32_t sample = readADC2Channel(adcChannels[ch]);
            uint8_t idx = voltageSampleIndex[ch];
            
            // If we have less than 10 samples, add to sum
            if (voltageSampleCount[ch] < VOLTAGE_FILTER_SAMPLES) {
                voltageSampleCount[ch]++;
                voltageSum[ch] += sample;
            } else {
                // Replace oldest sample in circular buffer
                voltageSum[ch] -= voltageSamples[ch][idx];
                voltageSum[ch] += sample;
            }
            
            // Store sample in circular buffer
            voltageSamples[ch][idx] = sample;
            voltageSampleIndex[ch] = (idx + 1) % VOLTAGE_FILTER_SAMPLES;
            
            // Calculate average (use last sample if not enough collected yet)
            if (voltageSampleCount[ch] > 0) {
                uint32_t avg = voltageSum[ch] / voltageSampleCount[ch];
                switch(ch) {
                    case 0: g_Update_DutVoltageA_uV = avg; break;
                    case 1: g_Update_DutVoltageB_uV = avg; break;
                    case 2: g_Update_DutVoltageC_uV = avg; break;
                    case 3: g_Update_DutVoltageD_uV = avg; break;
                }
            }
        }

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