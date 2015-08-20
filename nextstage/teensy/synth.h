#ifndef SYNTH_H_INCLUDED
#define SYNTH_H_INCLUDED 1

#include "common.h"

// Used a lot for fixed-point arithmetic.
#define UNIT     0x100000000LL

class ADSR {
    uint64_t _value;
    int64_t dvalue;
    uint32_t count;
    uint8_t _state;
    float attack, decay, sustain, release;

    void rare_step(void);

public:
    ADSR() {
        _state = _value = dvalue = count = 0;
    }

    void setA(float a);
    void setD(float d);
    void setS(float s);
    void setR(float r);

    uint32_t state() {
        return _state;
    }
    uint32_t output() {
        return _value >> 32;
    }
    void keydown(uint32_t down);
    void step(void);
};

class Filter {
    int32_t integrator1, integrator2, u;
    uint32_t w0dt, two_k, _f, _k;

    void compute_two_k(void);

public:
    Filter() {
        integrator1 = integrator2 = u = 0;
    }

    void setF(uint32_t f);
    void setQ(float q);
    void step(int32_t x);

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
    void setwaveform(int32_t x) {
        // 0 ramp, 1 triangle, 2 square
        waveform = x;
    }

    void step(void) {
        phase += dphase;
    }
    uint32_t get_phase(void) {
        return phase;
    }
    int32_t output(void);
};

#endif     // SYNTH_H_INCLUDED
