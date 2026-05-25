#include "lantan_demodulator.h"

#include "stm32h7xx_hal.h"
#include "lantan_synth.h"
#include "lantan_ll.h"
#include "lptim.h"
#include "gpio.h"
#include "adc.h"
#include "stm32h7xx_hal_def.h"

#define DEMOD_BUF_LEN   10000
uint32_t demodBuf[DEMOD_BUF_LEN];


static void vLocal_SetSamplingTimer(float samplingFreq) {
    
    // configure LPTIM1 to be 100kHz sampling clock for ADC1
    // TODO 
}   

static void vLocal_StartADC(void) {

    // set ADC1 to fill the demod buff in DMA mode
    // 16 bit resolution
    // TODO
}

float fDemod_SingleFreq(float _demodFreq, DemodSource_t _src, AD5664_Channel_t _diagCh) {
    
    // prepare peripherals
    // TODO

    // set diagnostic channel
    vLL_SetDACDiagnosticChannel(_diagCh);

    // set ADC channel (ADC1 IN3 - Detector, ADC1 IN10 - diagnostic)
    // TODO

    // record data into buffer in a blocking fashion, so just launch ADC and timer
    // and wait until DMA is done
    // TODO

    // demodulate at the selected _demodFreq frequency 
    // TODO

    // return calculated value
    // TODO
}