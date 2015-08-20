#ifndef SYNTH_H_INCLUDED
#define SYNTH_H_INCLUDED 1

#include <stdio.h>
#include <math.h>

#define UNIT     0x100000000LL
#define UNIT_2   0x200000000LL
#define UNITSQ   (1.0f * UNIT * UNIT)
#define GOAL     (0.95 * UNITSQ)

/* 1 / (1 - 1/e), because exponential */
#define BIGGER (1.5819767 * UNITSQ)

#define RARE  500

class ADSR {
    uint64_t _value;
    int64_t dvalue;
    uint32_t count;
    uint8_t _state;
    float attack, decay, sustain, release;

public:
    ADSR() {
        _state = _value = dvalue = count = 0;
    }
    void setA(float a) {
        if (a < 0.01) a = 0.01;
        attack = a;
    }
    void setD(float d) {
        if (d < 0.01) d = 0.01;
        decay = d;
    }
    void setS(float s) {
        sustain = GOAL * s;
    }
    void setR(float r) {
        if (r < 0.01) r = 0.01;
        release = r;
    }

    uint32_t state() {
        return _state;
    }
    uint32_t output() {
        return _value >> 32;
    }
    void keydown(uint32_t down) {
        if (down) {
            _state = 1;
            count = 0;
        } else {
            _state = 0;
        }
    }
    void rare_step(void) {
        // Done rarely, so float arithmetic OK here
        float next_value, h;
        switch (_state) {
        case 1:
            h = exp(-RARE * DT / attack);
            next_value = h * _value + (1.0 - h) * BIGGER;
            ASSERT(next_value - _value < UNITSQ);
            ASSERT(next_value - _value > -UNITSQ);
            if (next_value > GOAL) {
                _state = 2;
                next_value = GOAL;
            }
            break;
        case 2:
            h = exp(-RARE * DT / decay);
            next_value = h * _value + (1.0 - h) * sustain;
            break;
        default:
        case 0:
            h = exp(-RARE * DT / release);
            next_value = h * _value;
            if (next_value < 1)
                next_value = 0;
            break;
        }
        dvalue = 1. * (next_value - _value) / RARE;
    }
    void step(void) {
        if (count == 0) {
            rare_step();
        }
        count = (count + 1) % RARE;
        _value += dvalue;
    }
};

#define FMUL  ((uint32_t) (2 * 6.2831853 * DT * UNIT))

class Filter {
    int32_t integrator1, integrator2, u;
    uint32_t w0dt;
    uint32_t two_k;

public:
    Filter() {
        integrator1 = integrator2 = u = 0;
    }
    void setF(uint32_t f) {
        w0dt = FMUL * f;
        // w0dt ranges from 0 to 0.358
    }
    void setQ(float q) {
        float _k = 1.0 / q;
        if (_k < 0.18) _k = 0.18;   // stability
        two_k = (uint32_t) (2 * UNIT * _k);
        // two_k should be 0.36*UNIT or larger
    }
    void step(int32_t x) {
        u = x >> 2;
        u -= MULDIV32(two_k, integrator1);
        u -= integrator2;
        // u is in the range from -2**30 to +2**30
        integrator2 += MULDIV32(w0dt, integrator1);
        integrator1 += MULDIV32(w0dt, u);
    }
    int32_t highpass(void) {
        return clip(u);
    }
    int32_t bandpass(void) {
        return clip(integrator1);
    }
    int32_t lowpass(void) {
        return clip(integrator2);
    }
};

class Oscillator {
    uint32_t phase, dphase, waveform;

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
    uint32_t get_phase(void) {
        return phase;
    }
    int32_t output(void) {
        switch (waveform) {
        default:
        case 0:
            // ramp
            return (int32_t) phase;
        case 1:
            // triangle
            switch (phase >> 30) {
            case 0:
                return (phase << 1) & 0x7fffffff;
            case 1:
                return ~(phase << 1) & 0x7fffffff;
            case 2:
                return ~(phase << 1) | 0x80000000;
            default:
                return (phase << 1) | 0x80000000;
            }
            break;
        case 2:
            // square
            if (phase == 0) return 0;
            if (phase < 0x80000000) return 0x7fffffff;
            return -0x80000000;
            break;
        }
    }
};

#endif     // SYNTH_H_INCLUDED
