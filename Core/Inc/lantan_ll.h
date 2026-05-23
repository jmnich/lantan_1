#ifndef LANTAN_LL_H_
#define LANTAN_LL_H_

#include "stm32h7xx_hal.h"

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

void vLL_LockLEDs(uint8_t _lock);
void vLL_SetLED(LantanLED_t _led, LantanLEDMode_t _mode);

#endif // LANTAN_LL_H_
