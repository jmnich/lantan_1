#ifndef LANTAN_SYNTH_H_
#define LANTAN_SYNTH_H_

#include "lantan_ll.h"
#include "driver_ad5664.h"

typedef enum {
    SynthChannel_A = 0,
    SynthChannel_B = 1,
    SynthChannel_C = 2,
    SynthChannel_D = 3,
} LantanSynthCh_t;

extern const float synthFrequency[];

void vSynth_CalculateChannel(LantanSynthCh_t _ch, uint32_t _offset, uint32_t _pkpk);
void vSynth_SetChannelEnabled(LantanSynthCh_t _ch, uint8_t _enable);
uint8_t uSynth_StartSynth(void);
void vSynth_SynthTimerCallback(void);

#endif // LANTAN_SYNTH_H_