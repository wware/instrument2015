#ifndef VOICE_H_INCLUDED
#define VOICE_H_INCLUDED 1

#include "synth.h"

class Voice {
public:
    Oscillator osc1, osc2, osc3;
    ADSR adsr, adsr2;
#if USE_FILTER
    Filter filt;
    uint32_t _f;
#endif

    Voice() {
#if USE_FILTER
        filt.setQ(6);
#endif
        osc1.setwaveform(1);
        osc2.setwaveform(2);
        osc3.setwaveform(2);
        adsr.setA(0.03);
        adsr.setD(0.7);
        adsr.setS(0.4);
        adsr.setR(0.1);
        adsr2.setA(0.03);
        adsr2.setD(0.6);
        adsr2.setS(0.0);
        adsr2.setR(0.6);
    }
    void step(void) {
        osc1.step();
        osc2.step();
        osc3.step();
        adsr.step();
        adsr2.step();
#if USE_FILTER
        filt.setF(MULDIV32(_f, adsr2.output()));
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
        osc2.setfreq(f + small_random());
        osc3.setfreq(f / 2 + 0.5 * small_random());
    }
    void keydown(uint32_t down) {
        adsr.keydown(down);
        adsr2.keydown(down);
    }
    int32_t output(void) {
        int64_t x;
#if USE_FILTER
        x = filt.bandpass();
        x += filt.highpass() >> 1;
        x >>= 1;
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
