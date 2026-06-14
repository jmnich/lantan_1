#ifndef LANTAN_DEMOD_H_
#define LANTAN_DEMOD_H_

#include "driver_ad5664.h"

typedef enum {
    DemodSrc_Diagnostic,
    DemodSrc_Detector,
} DemodSource_t;

float fDemod_SingleFreq(float _demodFreq, DemodSource_t _src, AD5664_Channel_t _diagCh);
void vDemod_Quad(float * _outA, float * _outB, float * _outC, float * _outD, uint32_t  * _detectorOutOfRange);

#endif // LANTAN_DEMOD_H_
