/*
We have a 40 kHz or 50 kHz timer interrupt that handles sound
samples. First it writes the previously computed sample to the serial DAC.
Then it computes the sample for the next time.
*/

#if ! defined(__SERIAL)
#define __SERIAL 0
#endif

#if ! defined(__ARDUINO)
#define __ARDUINO 1
#include <TimerOne.h>
#endif

#include <string.h>
#include <stdarg.h>
#include <math.h>

#define SAMPLING_RATE   40000
#define DT (1.0 / SAMPLING_RATE)

#define ADSR_BITS   30
#define ADSR_MAX (1 << ADSR_BITS)

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
        attack = (uint32_t) (ADSR_MAX * DT / a);
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
            value += attack;
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
        }
        phase += dphase;
    }
    int32_t output(void) {
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
    int32_t signed_output(void) {
        /*
         * 12 bit signed output, both the Teensy and the Mac like this,
         * though when there are multiple voices this will need more thought.
         */
        return ((output() >> 20) + 0x800) & 0xFFF;
    }
};


Voice v[8];

int t = 0;

void setup() {
    int i;
#if __ARDUINO
#if __SERIAL
    Serial.begin(9600);
#endif
    analogWriteResolution(12);
    Timer1.initialize((int) (1000000 * DT));
    Timer1.attachInterrupt(compute_sample);
    delay(3000);
#endif
    for (i = 0; i < 8; i++) {
        v[i].setwaveform(0);
        v[i].keydown(1);
        v[i].setA(0.2);
        v[i].setD(0.3);
        v[i].setS(0.6);
        v[i].setR(0.6);
    }

    v[0].setfreq(440);
    v[1].setfreq(440 * 5 / 4);
    v[2].setfreq(440 * 3 / 2);
    v[3].setfreq(440 * 2);

    v[4].setfreq(440         + 4);
    v[5].setfreq(440 * 5 / 4 + 4);
    v[6].setfreq(440 * 3 / 2 + 4);
    v[7].setfreq(440 * 2     + 4);
}


uint32_t get_12_bit_value(void)
{
    int i;
    int64_t x = 0;
    for (i = 0; i < 8; i++)
        x += v[i].output();
    return ((x >> 23) + 0x800) & 0xFFF;
}

void compute_sample(void)
{
    int i;
#if __ARDUINO
    analogWrite(A14, get_12_bit_value());
#endif
    for (i = 0; i < 8; i++)
        v[i].step();
}

void loop() {
#if __ARDUINO
    int i;
    if (t++ < 60) {
#if __SERIAL
        char buf[20];
        sprintf(buf, "%08X", (unsigned int) v[0].adsr_level());
        Serial.print(buf);
        Serial.println();
#endif
        if (t == 50) {
            for (i = 0; i < 8; i++)
                v[i].keydown(0);
        }
    }

    delay(50);
#endif
}

