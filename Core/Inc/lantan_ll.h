#ifndef LANTAN_LL_H_
#define LANTAN_LL_H_

#include "stm32h7xx_hal.h"
#include "driver_ad5664.h"

// Reference voltage for ADC conversions (2500 mV)
extern const float LANTAN_ADC_VREF_mV;

typedef enum {
    LantanLED_Flt,
    LantanLED_Work,
    LantanLED_Run,
} LantanLED_t;

typedef enum {
    LantanLED_On,
    LantanLED_Off,
    LantanLED_Toggle,
} LantanLEDMode_t;

typedef enum {
    LantanCurrSrc_A,
    LantanCurrSrc_B,
    LantanCurrSrc_C,
    LantanCurrSrc_D,
} LantanCurrSrc_t;

typedef enum {
    LantanSrcReleased,
    LantanSrcLocked,
} LantanSrcRelease_t;

typedef enum {
    LantanSrcVolt5V,
    LantanSrcVolt9V,
} LantanSrcVoltage_t;

typedef enum {
    DetRange_100R,
    DetRange_1k,
    DetRange_10k,
    DetRange_100k,
} LantanDetectorRange_t;

typedef enum {
    DetGain_0_5,
    DetGain_1_0,
    DetGain_4_55,
    DetGain_10,
} LantanDetectorGain_t;

void vLL_DetectorConfigure(LantanDetectorRange_t _rng, LantanDetectorGain_t _gain);
float fLL_CurrentSourceVoltMeas(LantanCurrSrc_t _srcCh);
void vLL_CurrentSourceVoltage(LantanCurrSrc_t _srcCh, LantanSrcVoltage_t _voltage);
void vLL_CurrentSourceRelease(LantanCurrSrc_t _srcCh, LantanSrcRelease_t _release);
void vLL_LockLEDs(uint8_t _lock);
void vLL_SetLED(LantanLED_t _led, LantanLEDMode_t _mode);
void vLL_Set9VRail(uint8_t _enabled);
void vLL_SetDACDiagnosticChannel(AD5664_Channel_t _ch);

#endif // LANTAN_LL_H_
