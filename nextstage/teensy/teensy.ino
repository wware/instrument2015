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

/* 1 / (1 - 1/e), because exponential */
#define BIGGER 1.5819767

#define THRESHOLD 5

struct gpio_port * const _PORTA = (struct gpio_port *) 0x400FF000;
struct gpio_port * const _PORTB = (struct gpio_port *) 0x400FF040;
struct gpio_port * const _PORTC = (struct gpio_port *) 0x400FF080;
struct gpio_port * const _PORTD = (struct gpio_port *) 0x400FF0C0;
struct gpio_port * const _PORTE = (struct gpio_port *) 0x400FF100;

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

Voice v[8];

class Key {
    int id;
    int previous, current;
public:
    Key *predecessor;
    Key(int _id, int pitch) {
        id = _id;
        predecessor = NULL;
        previous = current = 0;
    }
    int state(void) {
        return current;
    }
    int keydown(void) {
        return current && !previous;
    }
    int keyup(void) {
        return !current && previous;
    }
    void check(void) {
        uint32_t X = 0, Y = 0, portc = (uint32_t) _PORTC, chip = id >> 3;
        digitalWrite(4, (id >> 2) & 1);
        digitalWrite(3, (id >> 1) & 1);
        digitalWrite(2, (id >> 0) & 1);
        digitalWrite(5, chip == 0);
        digitalWrite(6, chip == 1);
        digitalWrite(7, chip == 2);
        digitalWrite(8, chip == 3);
        digitalWrite(9, chip == 4);
        Y = 0;
        // drive with PTC 4, detect with PTC 6
        digitalWrite(10, LOW);
#if (__ARDUINO || ASSEMBLY_YNH_IO)
        asm volatile(
            "mov %0, #0x10"                 "\n"
            "str %0, [%2, #4]"              "\n"
            "step1:"                        "\n"
            "ldr %0, [%2, #16]"             "\n"
            "ands %0, %0, #0x40"            "\n"
            "bne step2"                     "\n"
            "add %1, %1, #1"                "\n"
            "b step1"                       "\n"
            "step2:"                        "\n"
            : "+r" (X), "+r" (Y), "+r" (portc)
        );
#endif
        // Do we need debouncing with a capacitive touch keyboard?
        previous = current;
        current = Y > THRESHOLD;
#if HW_DEBUG
        if (keydown()) {
            Serial.print("Key ");
            Serial.print(id);
            Serial.println(" pressed");
            if (id < 8) {
                v[id].keydown(1);
            }
        }
        if (keyup() && id < 8) {
            v[id].keydown(0);
        }
#endif
    }
};

struct gpio_port {
    uint32_t DOR;    // Data output register
    uint32_t SOR;    // Set output register
    uint32_t COR;    // Clear output register
    uint32_t TOR;    // Toggle output register
    uint32_t DIR;    // Data input register
    uint32_t DDR;    // Data direction register
};

Key *keyboard;    // head of linked list

void scan_keyboard(void) {
    Key *k = keyboard;
    while (k != NULL) {
        k = k->predecessor;
    }
}


void setup() {
    int i;
    Key *k, *kprev = NULL;
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
#endif
    for (i = 0; i < 8; i++) {
        v[i].setwaveform(0);
        v[i].setA(0.1);
        v[i].setD(0.4);
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

#if HW_DEBUG
    for (i = 0; i < 34; i++) {
        k = new Key(i, (i < 17) ? i : (i - 5));
        k->predecessor = kprev;
        kprev = k;
    }
    // TODO the left-hand keyboard
    // TODO read the softpots
    keyboard = k;
#endif
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
    scan_keyboard();
    // TODO reading soft pots
    // TODO mapping keys to reachable pitches
    // TODO assigning pitches to voices
    // TODO tracking key up and key down events
}
