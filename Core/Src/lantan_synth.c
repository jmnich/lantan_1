#include "lantan_synth.h"
#include <math.h>

#include "driver_ad5664.h"
#include "stm32h753xx.h"
#include "tim.h"
#include "spi.h"
#include "gpio.h"

#define CHCNT 4
#define SYNTH_BUF_LEN 4000

static uint8_t synthChannelActive[CHCNT] = {0};
static uint32_t synthBuffer[CHCNT][SYNTH_BUF_LEN] __attribute__((section(".freertos_heap")));
static uint32_t synthUsedSamples[CHCNT] = {0};

const uint32_t maxSampleFrequencyPerCh = 75000;
const uint32_t samplingFrequency = maxSampleFrequencyPerCh;
const uint32_t totalSPITransmissionsFrequency = samplingFrequency * 4;

// frequencies of sinewaves for different channels
const float synthFrequency[] = {2500,1000,1500,500};

void vSynth_CalculateChannel(LantanSynthCh_t _ch, uint32_t _offset, uint32_t _pkpk) {
    // Calculate number of samples for one full period
    uint32_t numSamples = (uint32_t)lroundf((float)samplingFrequency / synthFrequency[_ch]);

    AD5664_Channel_t ad_channel;
    if(_ch == SynthChannel_A) ad_channel = AD5664_CHANNEL_A;
    else if(_ch == SynthChannel_B) ad_channel = AD5664_CHANNEL_B;
    else if(_ch == SynthChannel_C) ad_channel = AD5664_CHANNEL_C;
    else if(_ch == SynthChannel_D) ad_channel = AD5664_CHANNEL_D;

    // Generate one period of sine wave
    for (uint32_t i = 0; i < numSamples; i++) {
        float phase = 2.0f * 3.1415926535f * synthFrequency[_ch] * (float)i / (float)samplingFrequency;
        float sampleValue = (float)_offset + ((float)_pkpk / 2.0f) * sinf(phase);
        synthBuffer[_ch][i] = (uint32_t)AD5664_VoltageToCode((uint32_t)lroundf(sampleValue));
        uint8_t *bytes = (uint8_t *)&synthBuffer[_ch][i];
        uint8_t ucAddr = (uint8_t)ad_channel; 
        bytes[2] = (0b010 << 3) | (ucAddr << 0);

        // swap LSB and cmd around so the buffer can be used directly by SPI
        uint8_t lsb = bytes[0];
        uint8_t msb = bytes[1];
        uint8_t cmd = bytes[2];

        bytes[0] = cmd;
        bytes[1] = msb;
        bytes[2] = lsb;
    }

    // Save number of used samples
    synthUsedSamples[_ch] = numSamples;
}

void vSynth_SetChannelEnabled(LantanSynthCh_t _ch, uint8_t _enable) {
    synthChannelActive[_ch] = _enable;
}

uint8_t uSynth_StartSynth(void) {
    
    uint8_t activeChCounter = 0;
    for(int i = 0; i < CHCNT; i++) if(synthChannelActive[i]) activeChCounter++;

    if(activeChCounter == 0) return 1; // fail - no active channels

    // all APB clock should be running at 240 MHz

    // configure and start timer 
    TIM_MasterConfigTypeDef sMasterConfig = {0};
    htim7.Instance = TIM7;
    htim7.Init.Prescaler = 0;//49;
    htim7.Init.CounterMode = TIM_COUNTERMODE_UP;
    uint16_t period = 240E6 / totalSPITransmissionsFrequency;
    htim7.Init.Period = period - 1;
    // htim7.Init.Period = 1199;
    htim7.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    HAL_TIM_Base_Init(&htim7);
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_ENABLE;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    HAL_TIMEx_MasterConfigSynchronization(&htim7, &sMasterConfig);
 
    // full spi transmission must fit between timer isr ticks

    /*
    Note: faster SPI transmission should be possible than 
    currently implemented. Going up to 100k sampling drops
    every second message and setting SPI above 20 MHz 
    completely breaks the transmission.
    */

    HAL_TIM_Base_Start_IT(&htim7);

    return 0; // pass
}

void vSynth_SynthTimerCallback(void)
{
    static LantanSynthCh_t nextChannel = SynthChannel_A;
    static uint32_t nextIndex[] = {0,0,0,0};

    // start SPI transmission
    HAL_GPIO_WritePin(SPI4_CS_GPIO_Port, SPI4_CS_Pin, GPIO_PIN_RESET);
    HAL_SPI_Transmit_IT(&hspi4, (uint8_t*)&synthBuffer[nextChannel][nextIndex[nextChannel]], 3);

    // increment or wrap index
    nextIndex[nextChannel] = (nextIndex[nextChannel] + 1) % synthUsedSamples[nextChannel];

    // next time do a different channel
    nextChannel = (nextChannel + 1) % (SynthChannel_D + 1);
}

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
  if (hspi->Instance == SPI4)
  {
    // pull CS high
    HAL_GPIO_WritePin(SPI4_CS_GPIO_Port, SPI4_CS_Pin, GPIO_PIN_SET);
  }
}