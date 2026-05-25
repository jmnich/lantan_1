#include "lantan_synth.h"
#include <math.h>

#include "driver_ad5664.h"
#include "stm32h753xx.h"
#include "tim.h"

#define CHCNT 4
#define SYNTH_BUF_LEN 4000

static uint8_t synthChannelActive[CHCNT] = {0};
static uint32_t synthBufferChA[SYNTH_BUF_LEN] __attribute__((section(".freertos_heap")));
static uint32_t synthBufferChB[SYNTH_BUF_LEN] __attribute__((section(".freertos_heap")));
static uint32_t synthBufferChC[SYNTH_BUF_LEN] __attribute__((section(".freertos_heap")));
static uint32_t synthBufferChD[SYNTH_BUF_LEN] __attribute__((section(".freertos_heap")));

static uint32_t synthAUsedSamples = 0;
static uint32_t synthBUsedSamples = 0;
static uint32_t synthCUsedSamples = 0;
static uint32_t synthDUsedSamples = 0;

const uint32_t maxSampleFrequencyPerCh = 50000;
const uint32_t samplingFrequency = maxSampleFrequencyPerCh;
const uint32_t totalSPITransmissionsFrequency = samplingFrequency * 4;

// frequencies of sinewaves for different channels
const float synthAFrequency = 800;
const float synthBFrequency = 1000;
const float synthCFrequency = 1200;
const float synthDFrequency = 1400;

void vSynth_CalculateChannel(LantanSynthCh_t _ch, uint32_t _offset, uint32_t _pkpk) {
    uint32_t *buffer = NULL;
    uint32_t *usedSamples = NULL;
    float frequency = 0.0f;

    // Select buffer, usedSamples pointer, and frequency based on channel
    switch (_ch) {
        case SynthChannel_A:
            buffer = synthBufferChA;
            usedSamples = &synthAUsedSamples;
            frequency = synthAFrequency;
            break;
        case SynthChannel_B:
            buffer = synthBufferChB;
            usedSamples = &synthBUsedSamples;
            frequency = synthBFrequency;
            break;
        case SynthChannel_C:
            buffer = synthBufferChC;
            usedSamples = &synthCUsedSamples;
            frequency = synthCFrequency;
            break;
        case SynthChannel_D:
            buffer = synthBufferChD;
            usedSamples = &synthDUsedSamples;
            frequency = synthDFrequency;
            break;
        default:
            return;
    }

    // Calculate number of samples for one full period
    uint32_t numSamples = (uint32_t)lroundf((float)samplingFrequency / frequency);

    AD5664_Channel_t ad_channel;
    if(_ch == SynthChannel_A) ad_channel = AD5664_CHANNEL_A;
    else if(_ch == SynthChannel_B) ad_channel = AD5664_CHANNEL_B;
    else if(_ch == SynthChannel_C) ad_channel = AD5664_CHANNEL_C;
    else if(_ch == SynthChannel_D) ad_channel = AD5664_CHANNEL_D;

    // Generate one period of sine wave
    for (uint32_t i = 0; i < numSamples; i++) {
        float phase = 2.0f * 3.1415926535f * frequency * (float)i / (float)samplingFrequency;
        float sampleValue = (float)_offset + ((float)_pkpk / 2.0f) * sinf(phase);
        buffer[i] = (uint32_t)lroundf(sampleValue);
        uint8_t *bytes = (uint8_t *)&buffer[i];
        uint8_t ucAddr = (uint8_t)ad_channel; // = (eChannel == AD5664_CHANNEL_ALL) ? AD5664_ADDR_ALL_DACS : eChannel;  
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
    *usedSamples = numSamples;
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
    htim7.Init.Prescaler = 0;
    htim7.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim7.Init.Period = 1199;
    htim7.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    HAL_TIM_Base_Init(&htim7);
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_ENABLE;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    HAL_TIMEx_MasterConfigSynchronization(&htim7, &sMasterConfig);
 
    // full spi transmission must fit between timer isr ticks

    // CS should be pulled up in spi finished isr

    // that way i get some deadtime between spi transmission to satisfy the 15ns interval

    // test with oscillosope how fast i can go in practice

    // TODO

    HAL_TIM_Base_Start_IT(&htim7);


    return 0; // pass
}

void vSynth_SynthTimerCallback(void)
{

}

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
  if (hspi->Instance == SPI4)
  {
    // Handle SPI1 transmission complete event here
  }
}