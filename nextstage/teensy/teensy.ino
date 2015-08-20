/*
We have a 40 kHz or 50 kHz timer interrupt that handles sound
samples. First it writes the previously computed sample to the serial DAC.
Then it computes the sample for the next time.
*/

#if ! defined(__ARDUINO)
#define __ARDUINO 1
#include <TimerOne.h>
#endif

#include <stdlib.h>
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

float small_random() {
    return -2.0 + 0.01 *
#if __ARDUINO
    random(400);
#else
    (rand() % 400);
#endif
}

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

    for (i = 0; i < NUM_KEYS; i++) {
        keyboard[i] = new Key();
        keyboard[i]->id = i;
        keyboard[i]->pitch = 440.0 * pow(1.0594631, i - 9);
    }
}


uint32_t get_12_bit_value(void)
{
    int i;
    int64_t x = 0;
    for (i = 0; i < NUM_VOICES; i++)
        x += v[i].output();
    x /= NUM_VOICES;
    return ((x >> 20) + 0x800) & 0xFFF;
}

/**
 * The timer interrupt takes audio samples from the queue and feeds
 * them to the 12-bit DAC. If there is a queue underrun, it turns on
 * the LED briefly but visibly.
 */
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

uint8_t read_key(uint32_t id)
{
    uint32_t Y = 0;
#if __ARDUINO
    uint32_t j = id, chip;
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
    while (!digitalReadFast(11)) Y++;
    digitalWrite(10, LOW);
#endif
    return Y;
}

/**
 * Run the step function on each of the voices. Sum their outputs to
 * get a 12-bit sample, and put the sample in the queue. If the queue
 * is full, then hang onto the sample to try to enqueue it again next
 * time.
 */
void compute_sample(void) {
    int i;
    static uint32_t x, again = 0;

    if (!again) {
        for (i = 0; i < NUM_VOICES; i++)
            v[i].step();
        x = get_12_bit_value();
    }
#if __ARDUINO
    cli();
#endif
    again = samples.write(x);
#if __ARDUINO
    sei();
#endif
}

/**
 * Arduino loop function
 * @todo Read soft pots
 * @todo Read the left-hand keyboard
 * @todo Create Pitch class
 * @todo Map keys to reachable pitches
 */
void loop(void) {
    int i;

    for (i = 0; i < NUM_KEYS; i++)
        keyboard[i]->check();
    for (i = 0; i < 64; i++)
        compute_sample();
}
