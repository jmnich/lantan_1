#include "lantan_demodulator.h"

#include "projdefs.h"
#include "stm32h7xx.h"
#include "stm32h7xx_hal.h"
#include "lantan_synth.h"
#include "lantan_ll.h"
#include "tim.h"
#include "gpio.h"
#include "adc.h"
#include "stm32h7xx_hal_adc.h"
#include "stm32h7xx_hal_adc_ex.h"
#include "stm32h7xx_hal_def.h"
#include "stm32h7xx_hal_tim.h"
#include "string.h"
#include "math.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"


#define DEMOD_BUF_LEN   10000
uint32_t demodBuf[DEMOD_BUF_LEN] __attribute__((section(".adc_buffers")));

static volatile uint8_t adcDmaComplete = 0;

// DMA transfer complete callback
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc) {
    if (hadc->Instance == ADC1) {
        adcDmaComplete = 1;
    }
}

static uint32_t current_arr = 0; // Store the current ARR value for TIM6

static void vLocal_SetSamplingTimer(float samplingFreq) {
    
    // configure TIM6 to be sampling clock for ADC1
    // you can have any sampling frequency for as long as it is 200 kHz
    uint32_t apb1_freq = 240E6; // MHz
    uint32_t prescaler = 9;
    uint32_t arr = 119;

    // Stop TIM6 if running
    HAL_TIM_Base_Stop(&htim6);
    
    // Configure TIM6
    TIM_HandleTypeDef htim6_local = htim6;
    htim6_local.Init.Prescaler = prescaler;
    htim6_local.Init.Period = arr;
    // htim6_local.Init.Prescaler = 9;
    // htim6_local.Init.Period = 239;
    htim6_local.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim6_local.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    HAL_TIM_Base_Init(&htim6_local);
    
    // Configure TIM6 TRGO to trigger ADC on update event
    TIM_MasterConfigTypeDef sMasterConfig = {0};
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    HAL_TIMEx_MasterConfigSynchronization(&htim6_local, &sMasterConfig);
    
    // Update the global handle
    htim6 = htim6_local;
}   

static void vLocal_InitADC(void) {
    // set ADC1 to fill the demod buff in DMA mode
    // 16 bit resolution
    
    // Reset the completion flag
    adcDmaComplete = 0;
    
    // Configure ADC1 for external trigger from TIM6
    hadc1.Instance = ADC1;
    hadc1.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV1;
    hadc1.Init.Resolution = ADC_RESOLUTION_16B;
    hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
    hadc1.Init.EOCSelection = ADC_EOC_SEQ_CONV;
    hadc1.Init.LowPowerAutoWait = DISABLE;
    hadc1.Init.ContinuousConvMode = DISABLE;
    hadc1.Init.NbrOfConversion = 1;
    hadc1.Init.DiscontinuousConvMode = DISABLE;
    hadc1.Init.ExternalTrigConv = ADC_EXTERNALTRIG_T6_TRGO;
    hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_RISING;
    hadc1.Init.ConversionDataManagement = ADC_CONVERSIONDATA_DMA_ONESHOT;
    hadc1.Init.Overrun = ADC_OVR_DATA_PRESERVED;
    hadc1.Init.LeftBitShift = ADC_LEFTBITSHIFT_NONE;
    hadc1.Init.OversamplingMode = ENABLE;
    hadc1.Init.Oversampling.Ratio = 4;
    hadc1.Init.Oversampling.RightBitShift = ADC_RIGHTBITSHIFT_2;
    hadc1.Init.Oversampling.TriggeredMode = ADC_TRIGGEREDMODE_SINGLE_TRIGGER;
    hadc1.Init.Oversampling.OversamplingStopReset = ADC_REGOVERSAMPLING_CONTINUED_MODE;
    
    // Re-initialize ADC with these settings
    HAL_ADC_Init(&hadc1);
    
    // STM32H7 ADC requires calibration before use
    HAL_ADCEx_Calibration_Start(&hadc1, ADC_CALIB_OFFSET_LINEARITY, ADC_SINGLE_ENDED);


    /* ADC1 DMA Init */
    /* ADC1 Init */
    DMA_HandleTypeDef hdma_adc1;
    hdma_adc1.Instance = DMA1_Stream0;
    hdma_adc1.Init.Request = DMA_REQUEST_ADC1;
    hdma_adc1.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_adc1.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_adc1.Init.MemInc = DMA_MINC_ENABLE;
    hdma_adc1.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    hdma_adc1.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
    hdma_adc1.Init.Mode = DMA_NORMAL;
    hdma_adc1.Init.Priority = DMA_PRIORITY_LOW;
    hdma_adc1.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&hdma_adc1) != HAL_OK)
    {
      Error_Handler();
    }

    /* ADC1 interrupt Init */
    HAL_NVIC_SetPriority(ADC_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(ADC_IRQn);
}

static void vLocal_StartADC(void) {    
    // Start ADC with DMA first - it will wait for external trigger
    HAL_ADC_Start_DMA(&hadc1, (uint32_t*)demodBuf, DEMOD_BUF_LEN);

    // Start TIM6 to generate periodic triggers
    HAL_TIM_Base_Start_IT(&htim6);
}

float fDemod_SingleFreq(float _demodFreq, DemodSource_t _src, AD5664_Channel_t _diagCh) {
    
    // prepare peripherals
    // Reset ADC and DMA state
    HAL_ADC_Stop_DMA(&hadc1);
    HAL_TIM_Base_Stop(&htim6);
    
    vLocal_InitADC();

    // Set sampling frequency to at least 2x the demodulation frequency
    // For proper sampling, we typically want 5-10x the frequency
    float samplingFreq = 2E5; // 200 kHz
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
    sConfig.SamplingTime = ADC_SAMPLETIME_32CYCLES_5;
    sConfig.SingleDiff = ADC_SINGLE_ENDED;
    sConfig.OffsetNumber = ADC_OFFSET_NONE;
    sConfig.Offset = 0;
    sConfig.OffsetSignedSaturation = DISABLE;
    HAL_ADC_ConfigChannel(&hadc1, &sConfig);

    // record data into buffer in a blocking fashion, so just launch ADC and timer
    // and wait until DMA is done
    vLocal_StartADC();
    
    // Wait for DMA completion with timeout
    // Add some margin: 3 seconds timeout
    uint32_t timeout = 3000;
    while (adcDmaComplete == 0 && timeout > 0) {
        timeout--;
        vTaskDelay(pdMS_TO_TICKS(1));
    }

    // Stop peripherals
    HAL_ADC_Stop_DMA(&hadc1);
    HAL_TIM_Base_Stop(&htim6);

    // Check if timeout occurred
    if (timeout == 0) {
        // return error
        return -1.0f; // Error: timeout
    }

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