#include "lantan_ll.h"

#include "driver_ad5664.h"
#include "gpio.h"
#include "main.h"
#include "stm32h753xx.h"
#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal_gpio.h"
#include <stdint.h>
#include "adc.h"

static uint8_t ledsLocked = 0;

LantanDetectorRange_t currentDetectorRange = 0; 
LantanDetectorGain_t currentDetectorGain = 0;

// Reference voltage for ADC conversions (2500 mV)
const float LANTAN_ADC_VREF_mV = 2500.0f;

void vLL_DetectorConfigure(LantanDetectorRange_t _rng, LantanDetectorGain_t _gain) {
    
    GPIO_PinState stateA;
    GPIO_PinState stateB;
    
    if(_rng == DetRange_100R) {
        stateA = GPIO_PIN_RESET;
        stateB = GPIO_PIN_RESET;
    } else if (_rng == DetRange_1k) {
        stateA = GPIO_PIN_SET;
        stateB = GPIO_PIN_RESET;
    } else if (_rng == DetRange_10k) {
        stateA = GPIO_PIN_RESET;
        stateB = GPIO_PIN_SET;
    } else if (_rng == DetRange_100k) {
        stateA = GPIO_PIN_SET;
        stateB = GPIO_PIN_SET;
    }

    HAL_GPIO_WritePin(TIA_SEL_A_GPIO_Port, TIA_SEL_A_Pin, stateA);
    HAL_GPIO_WritePin(TIA_SEL_B_GPIO_Port, TIA_SEL_B_Pin, stateB);

    if(_gain == DetGain_0_5) {
        stateA = GPIO_PIN_SET;
        stateB = GPIO_PIN_SET;
    } else if(_gain == DetGain_1_0) {
        stateA = GPIO_PIN_RESET;
        stateB = GPIO_PIN_SET;
    } else if(_gain == DetGain_4_55) {
        stateA = GPIO_PIN_SET;
        stateB = GPIO_PIN_RESET;        
    } else if(_gain == DetGain_10) {
        stateA = GPIO_PIN_RESET;
        stateB = GPIO_PIN_RESET;
    }

    HAL_GPIO_WritePin(AMP_SEL_A_GPIO_Port, AMP_SEL_A_Pin, stateA);
    HAL_GPIO_WritePin(AMP_SEL_B_GPIO_Port, AMP_SEL_B_Pin, stateB);

    currentDetectorRange = _rng;
    currentDetectorGain = _gain;
}

static void vlocal_LEDsOff(void) {
    HAL_GPIO_WritePin(LED0_DBG_GPIO_Port, LED0_DBG_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LED1_DBG_GPIO_Port, LED1_DBG_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LED2_DBG_GPIO_Port, LED2_DBG_Pin, GPIO_PIN_RESET);
    
    HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin, GPIO_PIN_RESET);
}

float fLL_CurrentSourceVoltMeas(LantanCurrSrc_t _srcCh) {
    ADC_ChannelConfTypeDef sConfig = {0};
    uint32_t adcChannel;
    uint32_t adcValue;
    float voltage_mV;

    // Select ADC channel based on current source channel
    switch(_srcCh) {
        case LantanCurrSrc_A:
            adcChannel = ADC_CHANNEL_7;  // ADC2 IN7
            break;
        case LantanCurrSrc_B:
            adcChannel = ADC_CHANNEL_4;  // ADC2 IN4
            break;
        case LantanCurrSrc_C:
            adcChannel = ADC_CHANNEL_8;  // ADC2 IN8
            break;
        case LantanCurrSrc_D:
            adcChannel = ADC_CHANNEL_9;  // ADC2 IN9
            break;
        default:
            return 0.0f;
    }

    // Configure ADC2 channel
    sConfig.Channel = adcChannel;
    sConfig.Rank = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
    sConfig.SingleDiff = ADC_SINGLE_ENDED;
    sConfig.OffsetNumber = ADC_OFFSET_NONE;
    sConfig.Offset = 0;
    sConfig.OffsetSignedSaturation = DISABLE;
    
    if (HAL_ADC_ConfigChannel(&hadc2, &sConfig) != HAL_OK) {
        return 0.0f;
    }

    // Start ADC conversion
    if (HAL_ADC_Start(&hadc2) != HAL_OK) {
        return 0.0f;
    }

    // Wait for conversion to complete
    if (HAL_ADC_PollForConversion(&hadc2, 10) != HAL_OK) {
        HAL_ADC_Stop(&hadc2);
        return 0.0f;
    }

    // Read ADC value
    adcValue = HAL_ADC_GetValue(&hadc2);
    
    // Stop ADC
    HAL_ADC_Stop(&hadc2);

    // Convert ADC value to voltage in mV
    // ADC is 16-bit: voltage = (adcValue * Vref) / (2^16 - 1)
    voltage_mV = (adcValue * LANTAN_ADC_VREF_mV) / 65535.0f;

    return voltage_mV;
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