/*
We have a 40 kHz or 50 kHz timer interrupt that handles sound
samples. First it writes the previously computed sample to the serial DAC.
Then it computes the sample for the next time.
*/

#define ASSEMBLY_YNH_IO 0
#if ASSEMBLY_YNH_IO
/*
 * I'm going to want to code stuff in assembly language.
 * Here are some hacks for http://assembly.ynh.io/ which is an online
 * ARM cross-compiler/assembler.
 */
#include <stdint.h>
#define __ARDUINO 0
#define HW_DEBUG 0
#endif

#if ! defined(HW_DEBUG)
#define HW_DEBUG 1
#endif

#if HW_DEBUG
#define __SERIAL 1
#endif

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

#define SAMPLING_RATE   20000
#define DT (1.0 / SAMPLING_RATE)

#define ADSR_BITS   30
#define ADSR_MAX (1 << ADSR_BITS)

#define NUM_KEYS 16   // 17? 34? 40?
#define NUM_VOICES 8

/* 1 / (1 - 1/e), because exponential */
#define BIGGER 1.5819767

// This is for a 330K resistor.
#define THRESHOLD 12

#define KEYDOWN_COUNT 12

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

Voice v[NUM_VOICES];
static uint8_t next_voice_to_assign = 0;

class Key;
Key *keyboard[NUM_KEYS];

class Key {
public:
    uint32_t id, state, count;
    float pitch;
    Voice *voice;
    Key() {
        count = state = 0;
        voice = NULL;
    }
    uint32_t check(void) {
        uint32_t Y = 0;
#if __ARDUINO
        uint32_t i, j = id, chip;
        digitalWrite(10, LOW);
        j = id;
        switch (id) {    // OOPS WIRING ERRORS
            case 0:
                j = 1;
                break;
            case 1:
                j = 0;
                break;
        }
        chip = j >> 3;
        digitalWrite(4, (j >> 2) & 1);
        digitalWrite(3, (j >> 1) & 1);
        digitalWrite(2, (j >> 0) & 1);
        digitalWrite(5, chip == 1);   // OOPS WIRING ERROR
        digitalWrite(6, chip == 0);
        digitalWrite(7, chip == 2);
        digitalWrite(8, chip == 3);
        digitalWrite(9, chip == 4);
        Y = 0;
        digitalWrite(10, HIGH);
        // while (!digitalRead(11)) Y++;
        while (!digitalReadFast(11)) Y++;
        digitalWrite(10, LOW);
#else
        Y = THRESHOLD + 1;
#endif
        if (Y > THRESHOLD) {
            if (state) {
                count = 0;
            } else {
                if (count < KEYDOWN_COUNT) {
                    count++;
                    if (count == KEYDOWN_COUNT) {
                        // this is a keydown event
                        state = 1;
                        count = 0;

                        i = next_voice_to_assign;
                        next_voice_to_assign = (i + 1) % NUM_VOICES;

                        // does some other key already have this voice? If so, remove
                        // the voice from that other key
                        for (j = 0; j < NUM_KEYS; j++)
                            if (j != id && keyboard[j]->voice == &v[i]) {
                                  keyboard[j]->voice = NULL;
                            }

                        voice = &v[i];
                        voice->setfreq(pitch);
                        voice->keydown(1);
                    }
                }
            }
        } else {
            if (!state) {
                count = 0;
            } else {
                if (count < KEYDOWN_COUNT) {
                    count++;
                    if (count == KEYDOWN_COUNT) {
                        // this is a keyup event
                        state = 0;
                        count = 0;
                        if (voice != NULL) {
                            voice->keydown(0);
                            voice = NULL;
                        }
                    }
                }
            }
        }
#if 0 && HW_DEBUG
        if (id == 0) {
            digitalWrite(LED_BUILTIN, state);
        }
#endif
        return Y;
    }
};


void setup() {
    int i;
#if __SERIAL
    Serial.begin(9600);
#endif
#if __ARDUINO
    analogWriteResolution(12);
    Timer1.initialize((int) (1000000 * DT));
    Timer1.attachInterrupt(compute_sample);
    pinMode(11, INPUT_PULLUP);
    pinMode(10, OUTPUT);
    digitalWrite(10, LOW);
    pinMode(2, OUTPUT);
    pinMode(3, OUTPUT);
    pinMode(4, OUTPUT);
    pinMode(5, OUTPUT);
    pinMode(6, OUTPUT);
    pinMode(7, OUTPUT);
    pinMode(8, OUTPUT);
    pinMode(9, OUTPUT);
    pinMode(LED_BUILTIN, OUTPUT);
#endif

    for (i = 0; i < NUM_VOICES; i++) {
        v[i].setwaveform(1);
        v[i].setA(0.05);
        v[i].setD(0.1);
        v[i].setS(0.4);
        v[i].setR(0.3);
    }

    for (i = 0; i < NUM_KEYS; i++) {
        keyboard[i] = new Key();
        keyboard[i]->id = i;
        keyboard[i]->pitch = 440.0 * pow(1.0594631, i - 9);
    }
    // TODO the left-hand keyboard
    // TODO read the softpots
}


uint32_t get_12_bit_value(void)
{
    int i;
    int64_t x = 0;
    for (i = 0; i < NUM_VOICES; i++)
        x += v[i].output();
    // return ((x >> 23) + 0x800) & 0xFFF;
    return ((x >> 24) + 0x800) & 0xFFF;
}

void compute_sample(void)
{
    int i;
#if __ARDUINO
    analogWrite(A14, get_12_bit_value());
#endif
    for (i = 0; i < NUM_VOICES; i++)
        v[i].step();
}


void loop() {
    int i;
    for (i = 0; i < NUM_KEYS; i++)
        keyboard[i]->check();

    // TODO reading soft pots
    // TODO mapping keys to reachable pitches
    // TODO assigning pitches to voices
    // TODO tracking key up and key down events
}
