/*
We have a 40 kHz or 50 kHz timer interrupt that handles sound
samples. First it writes the previously computed sample to the serial DAC.
Then it computes the sample for the next time.
*/

#if ! defined(__SERIAL)
#define __SERIAL 1
#include <TimerOne.h>
#endif

#include <string.h>
#include <stdarg.h>
#include <math.h>

#define SAMPLING_RATE   40000
#define DT (1.0 / SAMPLING_RATE)

#define ADSR_BITS   30
#define ADSR_MAX (1 << ADSR_BITS)

class ADSRParameters {
public:
    float _decay_in, _sustain_in;
    uint32_t attack, decay, sustain, release;

    void set_decay_sustain() {
        sustain = (uint32_t) (ADSR_MAX * _sustain_in);
        decay = (uint32_t)
            ((ADSR_MAX * DT) * ADSR_MAX /
             (_decay_in * (ADSR_MAX - _sustain_in)));
    }

    void setA(float a) {
        if (a < 0.01) a = 0.01;
        attack = (uint32_t) (ADSR_MAX * DT / a);
    }
    void setD(float d) {
        if (d < 0.01) d = 0.01;
        _decay_in = d;
        set_decay_sustain();
    }
    void setS(float s) {
        _sustain_in = s;
        set_decay_sustain();
    }
    void setR(float r) {
        if (r < 0.01) r = 0.01;
        release = (int32_t)(ADSR_MAX * exp(-DT / r));
    }
};

class Voice {
    ADSRParameters *params;
    uint32_t state, value;    // adsr
    uint32_t phase, dphase, waveform;  // oscillator
public:
    Voice(ADSRParameters *_params) {
        params = _params;
        waveform = 1;
    }
    ~Voice() { }
    void setfreq(float f) {
        dphase = (int32_t)(0x100000000 * f / SAMPLING_RATE);
    }
    void setwaveform(int32_t x) {   // 0 for ramp, 1 for triangle
        waveform = x;
    }

    int adsr_state() {
        return state;
    }
    int adsr_level() {
        return value;
    }
    void keydown(uint32_t down) {
        if (down)
            state = 1;
        else
            state = 0;
    }
    void step(void) {
        uint64_t x;
        if (state == 1) {
            // attack
            value += params->attack;
            if (value >= ADSR_MAX)
                state = 2;
        }
        else if (state == 2) {
            // decay
            value -= params->decay;
            if (value <= params->sustain) {
                // sustain
                value = params->sustain;
                state = 3;
            }
        }
        else if (state == 0) {
            // release
            x = value;
            value = (x * params->release) >> ADSR_BITS;
        }
        phase += dphase;
    }
    int32_t output(void) {
        int64_t x;
        if (waveform) {
            if (phase >= 0x80000000) {
                x = ~phase;
            } else {
                x = phase;
            }
        } else {
            x = phase >> 1;
        }
        x = (x & 0x7fffffff) - 0x40000000;
        /*
         * Running on the Mac, this spans the 32-bit range
         * of signed numbers, from nearly -0x80000000 to 0x7fffffff.
         */
        return (x * value) >> (ADSR_BITS - 1);
    }
};


ADSRParameters adsr;
Voice v(&adsr);


int t = 0;


void setup() {
#if __SERIAL
    Serial.begin(9600);
    analogWriteResolution(12);
    Timer1.initialize((int) (1000000 * DT));
    Timer1.attachInterrupt(compute_sample);
#endif
    delay(3000);
    v.setfreq(440);
    v.setwaveform(0);
    v.keydown(1);
    adsr.setA(0.1);
    adsr.setD(0.5);
    adsr.setS(0.2);
    adsr.setR(0.4);
}


void compute_sample(void)
{
    analogWrite(A14, ((v.output() >> 20) + 0x800) & 0xFFF);
    v.step();
}

void loop() {
    char buf[20];
#if __SERIAL
    if (t++ < 40) {
        Serial.print(v.adsr_state(), DEC);
        Serial.print(" ");
        sprintf(buf, "%08X", (unsigned int) adsr.sustain);
        Serial.print(buf);
        Serial.print(" ");
        sprintf(buf, "%08X", (unsigned int) v.adsr_level());
        Serial.print(buf);
        Serial.println();
        if (t == 20) v.keydown(0);
    }
    //delayMicroseconds(20000);
    delay(50);
#endif
}

