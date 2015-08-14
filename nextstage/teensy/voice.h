#ifndef VOICE_H_INCLUDED
#define VOICE_H_INCLUDED 1

#include <math.h>


class Voice {
    uint32_t state, value;    // adsr
    uint32_t attack, decay, sustain, release, gap;

    uint32_t phase, dphase, waveform;  // oscillator

public:

    Voice() {
        waveform = 1;
    }
    ~Voice() { }

    void setfreq(float f) {
        dphase = (int32_t)(0x100000000L * f / SAMPLING_RATE);
    }
    void setwaveform(int32_t x) {   // 0 for ramp, 1 for triangle
        waveform = x;
    }

    void setA(float a) {
        if (a < 0.01) a = 0.01;
        attack = (int32_t)(ADSR_MAX * exp(-DT / a));
    }
    void setD(float d) {
        if (d < 0.01) d = 0.01;
        decay = (int32_t)(ADSR_MAX * exp(-DT / d));
    }
    void setS(float s) {
        sustain = (uint32_t) (ADSR_MAX * s);
    }
    void setR(float r) {
        if (r < 0.01) r = 0.01;
        release = (int32_t)(ADSR_MAX * exp(-DT / r));
    }

    uint32_t adsr_state() {
        return state;
    }
    uint32_t adsr_level() {
        return value;
    }
    void keydown(uint32_t down) {
        if (down) {
            state = 1;
            gap = (uint32_t) (BIGGER * ADSR_MAX - value);
        } else {
            state = 0;
        }
    }
	int32_t signed_output(void) {
	    /*
	     * 12 bit signed output, both the Teensy and the Mac like this,
	     * though when there are multiple voices this will need more thought.
	     */
	    return ((output() >> 20) + 0x800) & 0xFFF;
	}

    void step(void);
    int32_t output(void);
};

#endif     // VOICE_H_INCLUDED
