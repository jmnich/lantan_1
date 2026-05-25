#ifndef LANTAN_DEMOD_H_
#define LANTAN_DEMOD_H_

#include "driver_ad5664.h"

typedef enum {
    DemodSrc_Diagnostic,
    DemodSrc_Detector,
} DemodSource_t;

float fDemod_SingleFreq(float _demodFreq, DemodSource_t _src, AD5664_Channel_t _diagCh);

#endif // LANTAN_DEMOD_H_
