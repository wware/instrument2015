/*
We have a 40 kHz or 50 kHz timer interrupt that handles sound
samples. First it writes the previously computed sample to the serial DAC.
Then it computes the sample for the next time.
*/

#if ! defined(__ARDUINO)
#define __ARDUINO 1
#include <TimerOne.h>
#endif

#include <string.h>
#include <stdarg.h>
#include <math.h>

#include "common.h"
#include "voice.h"
#include "key.h"
#include "queue.h"

Voice v[NUM_VOICES];
Key *keyboard[NUM_KEYS];
Queue samples;

void timer_interrupt(void);

void setup() {
    int i;
#if __ARDUINO
    analogWriteResolution(12);
    Timer1.initialize((int) (1000000 * DT));
    Timer1.attachInterrupt(timer_interrupt);
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
    return ((x >> (17 + NUM_VOICE_BITS)) + 0x800) & 0xFFF;
}

void timer_interrupt(void)
{
    static uint8_t led_time;
    uint32_t x;
    if (samples.read(&x) == 0) {
#if __ARDUINO
        analogWrite(A14, x);
#endif
    } else {
        led_time = 100;
    }
    if (led_time > 0) {
        led_time--;
#if __ARDUINO
        digitalWrite(LED_BUILTIN, HIGH);
#endif
    } else {
#if __ARDUINO
        digitalWrite(LED_BUILTIN, LOW);
#endif
    }
}

void compute_sample(void) {
    int i;
    static uint32_t x, again = 0;

    if (!again) {
        for (i = 0; i < NUM_VOICES; i++)
            v[i].step();
        x = get_12_bit_value();
    }
    again = samples.write(x);
}

void loop(void) {
    int i;

    for (i = 0; i < NUM_KEYS; i++)
        keyboard[i]->check();

    // TODO reading soft pots
    // TODO mapping keys to reachable pitches
    // TODO assigning pitches to voices
    // TODO tracking key up and key down events
}
