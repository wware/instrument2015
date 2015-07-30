/*
We have a 40 kHz or 50 kHz timer interrupt that handles sound
samples. First it writes the previously computed sample to the serial DAC.
Then it computes the sample for the next time.
*/

#if ! defined(__SERIAL)
#define __SERIAL 1
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


Voice v;


int t = 0;


void setup() {
#if __SERIAL
    Serial.begin(9600);
#endif
#if __ARDUINO
    analogWriteResolution(12);
    Timer1.initialize((int) (1000000 * DT));
    Timer1.attachInterrupt(compute_sample);
    delay(3000);
#endif
    v.setfreq(440);
    v.setwaveform(1);
    v.keydown(1);
    v.setA(0.2);
    v.setD(0.3);
    v.setS(0.6);
    v.setR(0.6);
}


void compute_sample(void)
{
#if __ARDUINO
    analogWrite(A14, v.signed_output());
#endif
    v.step();
}

void loop() {
#if __ARDUINO
    char buf[20];
    if (t++ < 40) {
#if __SERIAL
        Serial.print(v.adsr_state(), DEC);
        Serial.print(" ");
        sprintf(buf, "%08X", (unsigned int) v.adsr_level());
        Serial.print(buf);
        Serial.println();
#endif
        if (t == 20) v.keydown(0);
    }

    delay(50);
#endif
}

