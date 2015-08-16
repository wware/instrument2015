#ifndef VOICE_H_INCLUDED
#define VOICE_H_INCLUDED 1

#include "synth.h"

class Voice {
public:
    Oscillator osc1, osc2, osc3;
    ADSR adsr;

    Voice() {
        osc1.setwaveform(0);
        osc2.setwaveform(0);
        osc3.setwaveform(0);
        adsr.setA(0.03);
        adsr.setD(0.3);
        adsr.setS(0.4);
        adsr.setR(0.1);
    }
    void step(void) {
        osc1.step();
        osc2.step();
        osc3.step();
        adsr.step();
    }
    void setfreq(float f) {
        osc1.setfreq(f);
        osc2.setfreq(f + 2);
        osc3.setfreq(f / 2);
    }
    void keydown(uint32_t down) {
        adsr.keydown(down);
    }
    int32_t output(void) {
        int64_t x = osc1.output();
        x += osc2.output();
        x += osc3.output();
        return mult_unsigned_signed(adsr.output(), x >> 2);
    }
};

#endif     // VOICE_H_INCLUDED
