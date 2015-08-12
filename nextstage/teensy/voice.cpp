#include "common.h"
#include "voice.h"

void Voice::step(void) {
    uint64_t x;
    if (state == 1) {
        // attack
        value = (uint32_t) (BIGGER * ADSR_MAX - gap);
        x = gap;
        gap = (x * attack) >> ADSR_BITS;
        if (value >= ADSR_MAX) {
            state = 2;
            gap = value - sustain;
        }
    }
    else if (state == 2) {
        // decay
        value = gap + sustain;
        x = gap;
        gap = (x * decay) >> ADSR_BITS;
    }
    else if (state == 0) {
        // release
        x = value;
        value = (x * release) >> ADSR_BITS;
        gap = (uint32_t) (BIGGER * ADSR_MAX - value);
    }
    phase += dphase;
}

int32_t Voice::output(void) {
    int64_t x = 0;
    switch (waveform) {
    case 0:
        // ramp
        x = phase >> 1;
        break;
    case 1:
        // triangle
        if (phase >= 0x80000000) {
            x = ~phase;
        } else {
            x = phase;
        }
        break;
    case 2:
        // square
        if (phase >= 0x80000000) {
            x = 0x7fffffff;
        } else {
            x = -0x80000000;
        }
        break;
    }
    x = (x & 0x7fffffff) - 0x40000000;
    /*
     * Running on the Mac, this spans the 32-bit range
     * of signed numbers, from nearly -0x80000000 to 0x7fffffff.
     */
    return (x * value) >> (ADSR_BITS - 1);
}
