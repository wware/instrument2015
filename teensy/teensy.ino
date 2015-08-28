/*
We have a 40 kHz or 50 kHz timer interrupt that handles sound
samples. First it writes the previously computed sample to the serial DAC.
Then it computes the sample for the next time.
*/

#include <TimerOne.h>

#include "common.h"
#include "voice.h"
#include "key.h"

Key *keyboard[NUM_KEYS];

class ThreadSafeSynth : public Synth
{
    void write_sample(void) {
        cli();
        Synth::write_sample();
        sei();
    }
};

ThreadSafeSynth s;
ISynth *synth = &s;

/**
 * The timer interrupt takes audio samples from the queue and feeds
 * them to the 12-bit DAC. If there is a queue underrun, it turns on
 * the LED briefly but visibly.
 */
void timer_interrupt(void)
{
    static uint8_t led_time;
    uint32_t x;
    if (s.get_sample(&x) == 0) {
        analogWrite(A14, x);
    } else {
        led_time = 100;
    }
    if (led_time > 0) {
        led_time--;
        digitalWrite(LED_BUILTIN, HIGH);
    } else {
        digitalWrite(LED_BUILTIN, LOW);
    }
}


void setup() {
    int i;
    for (i = 0; i < NUM_VOICES; i++)
        s.add(new Voice());
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

    for (i = 0; i < NUM_KEYS; i++) {
        keyboard[i] = new Key();
        keyboard[i]->id = i;
        keyboard[i]->pitch = i;
    }
}

uint8_t read_key(uint32_t id)
{
    uint32_t Y = 0;
    uint32_t j = id, chip;
    digitalWrite(10, LOW);
    j = id;
    chip = j >> 3;
    digitalWrite(4, (j >> 2) & 1);
    digitalWrite(3, (j >> 1) & 1);
    digitalWrite(2, (j >> 0) & 1);
    digitalWrite(5, chip == 0);
    digitalWrite(6, chip == 1);
    digitalWrite(7, chip == 2);
    digitalWrite(8, chip == 3);
    digitalWrite(9, chip == 4);
    Y = 0;
    digitalWrite(10, HIGH);
    while (!digitalReadFast(11)) Y++;
    digitalWrite(10, LOW);
    return Y;
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
        s.compute_sample();
}
