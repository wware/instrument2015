#ifndef VOICE_H_INCLUDED
#define VOICE_H_INCLUDED 1

#include "synth.h"

class Voice {
public:
    Oscillator osc1, osc2, osc3;
    ADSR adsr;
#if USE_FILTER
    Filter filt;
    double _f;
#endif

    Voice() {
#if USE_FILTER
        filt.setQ(4);
#endif
        osc1.setwaveform(0);
        osc2.setwaveform(0);
        osc3.setwaveform(0);
        adsr.setA(0.03);
        adsr.setD(0.7);
        adsr.setS(0.4);
        adsr.setR(0.1);
    }
    void step(void) {
        osc1.step();
        osc2.step();
        osc3.step();
        adsr.step();
#if USE_FILTER
        filt.setF(_f * adsr.output() / (1UL << 32));
        int64_t x = osc1.output();
        x += osc2.output();
        x += osc3.output();
        filt.step(x >> 1);
#endif
    }
    void setfreq(float f) {
#if USE_FILTER
        _f = f;
#endif
        osc1.setfreq(f);
        osc2.setfreq(f + 2);
        osc3.setfreq(f / 2);
    }
    void keydown(uint32_t down) {
        adsr.keydown(down);
    }
    int32_t output(void) {
        int64_t x;
#if USE_FILTER
        x = filt.bandpass();
#else
        x = osc1.output();
        x += osc2.output();
        x += osc3.output();
        x >>= 2;
#endif
        return mult_unsigned_signed(adsr.output(), x);
    }
};

#endif     // VOICE_H_INCLUDED
