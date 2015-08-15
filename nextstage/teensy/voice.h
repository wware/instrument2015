#ifndef VOICE_H_INCLUDED
#define VOICE_H_INCLUDED 1

#include <stdio.h>
#include <math.h>

#define UNIT   0x100000000LL

/* 1 / (1 - 1/e), because exponential */
#define BIGGER ((uint64_t) (1.5819767 * UNIT))

class ADSR {
    uint32_t _state, _value;    // adsr
    uint32_t attack, decay, sustain, release;
    uint64_t gap;

public:
    ADSR() {
        _state = _value = 0;
    }
    void setA(float a) {
        if (a < 0.01) a = 0.01;
        // attack = (int32_t)(UNIT * exp(-DT / a));
        // Switching to LINEAR attack
        attack = (uint32_t) (DT * UNIT / a);
    }
    void setD(float d) {
        if (d < 0.01) d = 0.01;
        decay = (uint32_t)(UNIT * exp(-DT / d));
    }
    void setS(float s) {
        sustain = (uint32_t) (UNIT * s);
    }
    void setR(float r) {
        if (r < 0.01) r = 0.01;
        release = (uint32_t)(UNIT * exp(-DT / r));
    }

    uint32_t state() {
        return _state;
    }
    uint32_t output() {
        return _value;
    }
    void keydown(uint32_t down) {
        if (down) {
            _state = 1;
        } else {
            _state = 0;
        }
    }
    void step(void) {
        uint64_t x;
        if (_state == 1) {
            // attack
            x = ((uint64_t) _value) + attack;
            if (x >= UNIT) {
                _state = 2;
                _value = UNIT - 1;
                gap = _value - sustain;
            } else {
                _value = x;
            }
        }
        else if (_state == 2) {
            // decay
            _value = gap + sustain;
            x = gap;
            gap = (x * decay) >> 32;
        }
        else if (_state == 0) {
            // release
            x = _value;
            if (_value < (1 << 18))
                // fix for a fixed-point bug where notes
                // never really completely end
                _value = 0;
            else
                _value = (x * release) >> 32;
        }
    }
};

class Oscillator {
    uint32_t phase, dphase, waveform;  // oscillator

public:
    Oscillator() {
        waveform = 1;
    }

    void setfreq(float f) {
        dphase = (int32_t)(UNIT * f / SAMPLING_RATE);
    }
    void setwaveform(int32_t x) {   // 0 for ramp, 1 for triangle
        waveform = x;
    }

    void step(void) {
        phase += dphase;
    }
    int32_t output(void) {
        int64_t x = 0;
        switch (waveform) {
        default:
        case 0:
            // ramp
            x = phase;
            return x - 0x80000000;
            break;
        case 1:
            // triangle
            if (phase >= 0x80000000) {
                x = ~phase;
            } else {
                x = phase;
            }
            return (x << 1) - 0x80000000;
            break;
        case 2:
            // square
            if (phase >= 0x80000000) {
                return 0x7fffffff;
            } else {
                return -0x80000000;
            }
            break;
        }
    }
};

class Voice {
public:
    Oscillator osc1, osc2, osc3;
    ADSR adsr;

    Voice() {
        osc1.setwaveform(1);
        osc2.setwaveform(1);
        osc3.setwaveform(1);
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
        osc2.setfreq(f + 3);
        osc3.setfreq(f - 5);
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
