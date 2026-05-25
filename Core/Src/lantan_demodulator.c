#include "lantan_demodulator.h"

#include "stm32h7xx_hal.h"
#include "lantan_synth.h"
#include "lantan_ll.h"
#include "lptim.h"
#include "gpio.h"
#include "adc.h"
#include "stm32h7xx_hal_def.h"
#include "string.h"
#include "math.h"

#define DEMOD_BUF_LEN   10000
uint32_t demodBuf[DEMOD_BUF_LEN];

static volatile uint8_t adcDmaComplete = 0;

// DMA transfer complete callback
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc) {
    if (hadc->Instance == ADC1) {
        adcDmaComplete = 1;
    }
}

static uint32_t current_arr = 0; // Store the current ARR value for LPTIM1

static void vLocal_SetSamplingTimer(float samplingFreq) {
    
    // configure LPTIM1 to be sampling clock for ADC1
    // APB1 clock is 100 MHz (from SystemClock_Config: APB1_DIV2 with HCLK=200MHz)
    // LPTIM1 uses D2PCLK1 which is APB1 = 100 MHz
    // 
    // To get samplingFreq, we need:
    //   timer_freq = 100MHz / (prescaler + 1) / (arr + 1) = samplingFreq
    // 
    // For 100kHz sampling: arr = (100MHz / prescaler) / 100kHz - 1
    // With prescaler = 0 (no prescaler), arr = 100000000 / 100000 - 1 = 999
    
    uint32_t apb1_freq = 100000000; // 100 MHz
    uint32_t prescaler = 0; // LPTIM_PRESCALER_DIV1
    current_arr = (apb1_freq / (prescaler + 1)) / (uint32_t)samplingFreq - 1;
    
    // Stop LPTIM1 if running
    HAL_LPTIM_Counter_Stop(&hlptim1);
    
    // Configure LPTIM1
    LPTIM_HandleTypeDef hlptim1_local = hlptim1;
    hlptim1_local.Init.Clock.Source = LPTIM_CLOCKSOURCE_APBCLOCK_LPOSC;
    hlptim1_local.Init.Clock.Prescaler = LPTIM_PRESCALER_DIV1;
    hlptim1_local.Init.Trigger.Source = LPTIM_TRIGSOURCE_SOFTWARE;
    hlptim1_local.Init.OutputPolarity = LPTIM_OUTPUTPOLARITY_HIGH;
    hlptim1_local.Init.UpdateMode = LPTIM_UPDATE_IMMEDIATE;
    hlptim1_local.Init.CounterSource = LPTIM_COUNTERSOURCE_INTERNAL;
    hlptim1_local.Init.Input1Source = LPTIM_INPUT1SOURCE_GPIO;
    hlptim1_local.Init.Input2Source = LPTIM_INPUT2SOURCE_GPIO;
    
    if (HAL_LPTIM_Init(&hlptim1_local) != HAL_OK) {
        Error_Handler();
    }
    
    // Update the global handle
    hlptim1 = hlptim1_local;
}   

static void vLocal_StartADC(void) {

    // set ADC1 to fill the demod buff in DMA mode
    // 16 bit resolution
    
    // Reset the completion flag
    adcDmaComplete = 0;
    
    // Configure ADC1 for external trigger from LPTIM1
    hadc1.Init.ExternalTrigConv = ADC_EXTERNALTRIG_LPTIM1_OUT;
    hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_RISING;
    hadc1.Init.ContinuousConvMode = DISABLE; // Single conversion per trigger
    
    if (HAL_ADC_Init(&hadc1) != HAL_OK) {
        Error_Handler();
    }
    
    // Start ADC with DMA
    if (HAL_ADC_Start_DMA(&hadc1, (uint32_t*)demodBuf, DEMOD_BUF_LEN) != HAL_OK) {
        Error_Handler();
    }
    
    // Start LPTIM1 in PWM mode to generate periodic triggers
    // Period = current_arr + 1 (autoreload value), Pulse = (current_arr + 1) / 2 (50% duty cycle)
    if (HAL_LPTIM_PWM_Start(&hlptim1, current_arr + 1, (current_arr + 1) / 2) != HAL_OK) {
        Error_Handler();
    }
}

float fDemod_SingleFreq(float _demodFreq, DemodSource_t _src, AD5664_Channel_t _diagCh) {
    
    // prepare peripherals
    // Reset ADC and DMA state
    HAL_ADC_Stop_DMA(&hadc1);
    HAL_LPTIM_Counter_Stop(&hlptim1);
    
    // Set sampling frequency to at least 2x the demodulation frequency
    // For proper sampling, we typically want 5-10x the frequency
    float samplingFreq = _demodFreq * 10.0f;
    // Limit sampling frequency to a reasonable range
    if (samplingFreq > 1000000.0f) {
        samplingFreq = 1000000.0f;
    }
    if (samplingFreq < 10000.0f) {
        samplingFreq = 10000.0f;
    }
    
    vLocal_SetSamplingTimer(samplingFreq);

    // set diagnostic channel
    vLL_SetDACDiagnosticChannel(_diagCh);

    // set ADC channel (ADC1 IN3 - Detector, ADC1 IN10 - diagnostic)
    ADC_ChannelConfTypeDef sConfig = {0};
    
    if (_src == DemodSrc_Detector) {
        sConfig.Channel = ADC_CHANNEL_3;  // PA6 - DETECTOR
    } else {
        sConfig.Channel = ADC_CHANNEL_10; // PC0 - DAC_DIAG
    }
    
    sConfig.Rank = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
    sConfig.SingleDiff = ADC_SINGLE_ENDED;
    sConfig.OffsetNumber = ADC_OFFSET_NONE;
    sConfig.Offset = 0;
    sConfig.OffsetSignedSaturation = DISABLE;
    
    if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK) {
        Error_Handler();
    }

    // record data into buffer in a blocking fashion, so just launch ADC and timer
    // and wait until DMA is done
    vLocal_StartADC();
    
    // Wait for DMA completion
    while (adcDmaComplete == 0) {
        // Wait for completion
    }
    
    // Stop peripherals
    HAL_ADC_Stop_DMA(&hadc1);
    HAL_LPTIM_Counter_Stop(&hlptim1);

    // demodulate at the selected _demodFreq frequency 
    // Digital quadratic demodulation:
    // For a signal s[n] = A * cos(2*pi*f*n/Ts + phi) + noise
    // We multiply by cos(2*pi*f*n/Ts) and sin(2*pi*f*n/Ts) and low-pass filter
    // 
    // In-phase (I): I = sum(s[n] * cos(2*pi*f*n/Ts))
    // Quadrature (Q): Q = sum(s[n] * sin(2*pi*f*n/Ts))
    // Intensity = sqrt(I^2 + Q^2) / N
    
    float Ts = 1.0f / samplingFreq;  // Sampling period
    float omega = 2.0f * 3.1415926535f * _demodFreq * Ts;  // Digital frequency
    
    float I = 0.0f;  // In-phase component
    float Q = 0.0f;  // Quadrature component
    
    uint32_t numSamples = DEMOD_BUF_LEN;
    
    for (uint32_t n = 0; n < numSamples; n++) {
        // Get ADC value (16-bit, stored in 32-bit buffer)
        // ADC value is in bits 0-15 of the 32-bit word
        uint16_t adcValue = (uint16_t)(demodBuf[n] & 0xFFFF);
        
        // Convert to float and normalize to [-1, 1] range
        // ADC is 16-bit, so full scale is 65535
        float signal = (float)adcValue / 32767.5f - 1.0f;  // Scale to [-1, 1]
        
        // Calculate cosine and sine for this sample
        float cos_val = cosf(n * omega);
        float sin_val = sinf(n * omega);
        
        // Accumulate I and Q
        I += signal * cos_val;
        Q += signal * sin_val;
    }
    
    // Calculate intensity (magnitude)
    // Intensity is proportional to sqrt(I^2 + Q^2)
    // Normalize by number of samples
    float intensity = sqrtf(I * I + Q * Q) / (float)numSamples;
    
    // Since we scaled the signal to [-1, 1], the maximum intensity would be 1.0
    // But ADC values are always positive (0 to 65535), so we need to adjust
    // Actually, for a pure tone at the demodulation frequency, we expect:
    // intensity = A/2 where A is the amplitude (as a fraction of full scale)
    // So we multiply by 2 to get the amplitude
    intensity *= 2.0f;
    
    // return calculated value
    return intensity;
}