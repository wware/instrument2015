#include <math.h>
#include "synth.h"

#define UNIT_2   0x200000000LL
#define UNITSQ   (1.0f * UNIT * UNIT)
#define GOAL     (0.95 * UNITSQ)

/* 1 / (1 - 1/e), because exponential */
#define BIGGER (1.5819767 * UNITSQ)

#define RARE  500
#define FMUL  ((uint32_t) (2 * 6.2831853 * DT * UNIT))
#define NOT_TOO_SMALL(x)    MAX(x, 0.01)

void ADSR::rare_step(void) {
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

void ADSR::setA(float a) {
    attack = NOT_TOO_SMALL(a);
}

void ADSR::setD(float d) {
    decay = NOT_TOO_SMALL(d);
}

void ADSR::setS(float s) {
    sustain = GOAL * s;
}

void ADSR::setR(float r) {
    release = NOT_TOO_SMALL(r);
}

void ADSR::keydown(uint32_t down) {
    if (down) {
        _state = 1;
        count = 0;
    } else {
        _state = 0;
    }
}

void ADSR::step(void) {
    if (count == 0) {
        rare_step();
    }
    count = (count + 1) % RARE;
    _value += dvalue;
}

int32_t Oscillator::output(void) {
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

void Filter::compute_two_k(void) {
    // k needs to scale with frequeuncy
    two_k = (UNIT_2 * _k * _f) >> 24;
}

void Filter::setF(uint32_t f) {
    _f = f;
    w0dt = FMUL * f;
    // w0dt ranges from 0 to 0.358
    compute_two_k();
}

void Filter::setQ(float q) {
    const float kmin = 0.1;
    float _fk;
    _fk = 1.0 / q;
    if (_fk < kmin) _fk = kmin;   // stability
    _k = _fk * (1 << 16);
    compute_two_k();
}

void Filter::step(int32_t x) {
    int64_t y = x >> 2;
    y -= MULDIV32(two_k, integrator1);
    y -= integrator2;
    // u is in the range from -2**30 to +2**30
    integrator2 = ADDCLIP(integrator2, MULDIV32(w0dt, integrator1));
    integrator1 = ADDCLIP(integrator1, MULDIV32(w0dt, u));
    u = clip(y);
}
