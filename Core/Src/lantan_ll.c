#include "lantan_ll.h"

#include "driver_ad5664.h"
#include "gpio.h"
#include "main.h"
#include "stm32h753xx.h"
#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal_gpio.h"
#include <stdint.h>

static uint8_t ledsLocked = 0;

static void vlocal_LEDsOff(void)
{
    HAL_GPIO_WritePin(LED0_DBG_GPIO_Port, LED0_DBG_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LED1_DBG_GPIO_Port, LED1_DBG_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LED2_DBG_GPIO_Port, LED2_DBG_Pin, GPIO_PIN_RESET);
    
    HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin, GPIO_PIN_RESET);
}

void vLL_CurrentSourceVoltage(LantanCurrSrc_t _srcCh, LantanSrcVoltage_t _voltage) {

    GPIO_TypeDef * port;
    uint16_t pin;
    GPIO_PinState state;

    if(_srcCh == LantanCurrSrc_A) {
        port = SRC_A_VOLT_SEL_GPIO_Port;
        pin = SRC_A_VOLT_SEL_Pin;
    } else if(_srcCh == LantanCurrSrc_B) {
        port = SRC_B_VOLT_SEL_GPIO_Port;
        pin = SRC_B_VOLT_SEL_Pin;
    } else if(_srcCh == LantanCurrSrc_C) {
        port = SRC_C_VOLT_SEL_GPIO_Port;
        pin = SRC_C_VOLT_SEL_Pin;
    } else {
        port = SRC_D_VOLT_SEL_GPIO_Port;
        pin = SRC_D_VOLT_SEL_Pin;
    }

    state = _voltage == LantanSrcVolt5V ? GPIO_PIN_RESET : GPIO_PIN_SET;
    HAL_GPIO_WritePin(port, pin, state);
}

void vLL_CurrentSourceRelease(LantanCurrSrc_t _srcCh, LantanSrcRelease_t _release) {
    
    GPIO_TypeDef * port;
    uint16_t pin;
    GPIO_PinState state;

    if(_srcCh == LantanCurrSrc_A) {
        port = SRC_A_RELEASE_GPIO_Port;
        pin = SRC_A_RELEASE_Pin;
    } else if(_srcCh == LantanCurrSrc_B) {
        port = SRC_B_RELEASE_GPIO_Port;
        pin = SRC_B_RELEASE_Pin;
    } else if(_srcCh == LantanCurrSrc_C) {
        port = SRC_C_RELEASE_GPIO_Port;
        pin = SRC_C_RELEASE_Pin;
    } else {
        port = SRC_D_RELEASE_GPIO_Port;
        pin = SRC_D_RELEASE_Pin;
    }

    state = _release == LantanSrcReleased ? GPIO_PIN_SET : GPIO_PIN_RESET;
    HAL_GPIO_WritePin(port, pin, state);
}

void vLL_LockLEDs(uint8_t _lock) {
    if(_lock) {
        ledsLocked = 1;
        vlocal_LEDsOff();
    }
    else ledsLocked = 0;
}

void vLL_SetLED(LantanLED_t _led, LantanLEDMode_t _mode) {

    if(ledsLocked) {
        vlocal_LEDsOff();
        return;
    }

    GPIO_TypeDef * GPIOx;
    uint16_t GPIO_Pin;
    GPIO_TypeDef * GPIOxDBG;
    uint16_t GPIO_PinDBG;

    if(_led == LantanLED_Flt) {
        GPIOx = LED1_GPIO_Port;
        GPIO_Pin = LED1_Pin;
        GPIOxDBG = LED0_DBG_GPIO_Port;
        GPIO_PinDBG = LED0_DBG_Pin;
    } else if(_led == LantanLED_Work) {
        GPIOx = LED2_GPIO_Port;
        GPIO_Pin = LED2_Pin;
        GPIOxDBG = LED1_DBG_GPIO_Port;
        GPIO_PinDBG = LED1_DBG_Pin;
    } else {
        GPIOx = LED3_GPIO_Port;
        GPIO_Pin = LED3_Pin;
        GPIOxDBG = LED2_DBG_GPIO_Port;
        GPIO_PinDBG = LED2_DBG_Pin;
    }

    if(_mode == LantanLED_On) {
        HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOxDBG, GPIO_PinDBG, GPIO_PIN_SET);
    } else if(_mode == LantanLED_Toggle) {
        HAL_GPIO_TogglePin(GPIOx, GPIO_Pin);
        HAL_GPIO_TogglePin(GPIOxDBG, GPIO_PinDBG);
    } else {
        HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOxDBG, GPIO_PinDBG, GPIO_PIN_RESET);
    }
}

void vLL_Set9VRail(uint8_t _enabled) {
    if(_enabled == 1) {
        HAL_GPIO_WritePin(ENA_9V_GPIO_Port, ENA_9V_Pin, GPIO_PIN_SET);
    } else {
        HAL_GPIO_WritePin(ENA_9V_GPIO_Port, ENA_9V_Pin, GPIO_PIN_RESET);
    }
}

void vLL_SetDACDiagnosticChannel(AD5664_Channel_t _ch) {
    switch(_ch)
    {
        case AD5664_CHANNEL_A:
            HAL_GPIO_WritePin(DAC_DIAG_A0_GPIO_Port, DAC_DIAG_A0_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(DAC_DIAG_A1_GPIO_Port, DAC_DIAG_A1_Pin, GPIO_PIN_RESET);
            break;
        case AD5664_CHANNEL_B:
            HAL_GPIO_WritePin(DAC_DIAG_A0_GPIO_Port, DAC_DIAG_A0_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(DAC_DIAG_A1_GPIO_Port, DAC_DIAG_A1_Pin, GPIO_PIN_RESET);
            break;
        case AD5664_CHANNEL_C:
            HAL_GPIO_WritePin(DAC_DIAG_A0_GPIO_Port, DAC_DIAG_A0_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(DAC_DIAG_A1_GPIO_Port, DAC_DIAG_A1_Pin, GPIO_PIN_SET);
            break;
        case AD5664_CHANNEL_D:
            HAL_GPIO_WritePin(DAC_DIAG_A0_GPIO_Port, DAC_DIAG_A0_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(DAC_DIAG_A1_GPIO_Port, DAC_DIAG_A1_Pin, GPIO_PIN_SET);
            break;
        default:
            break;        
    }
}