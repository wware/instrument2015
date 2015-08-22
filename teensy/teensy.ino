#include <TimerOne.h>

#include "common.h"
#include "voice.h"
#include "key.h"

#define NUM_KEYS 10   // 17? 34? 40?
#define NUM_VOICES 3

Key *keyboard[NUM_KEYS];

class ThreadSafeSynth : public Synth
{
    void write_sample(void) {
        cli();
        Synth::write_sample();
        sei();
    }
};

ThreadSafeSynth s, s2;
ISynth *synth;

/**
 * The timer interrupt takes audio samples from the queue and feeds
 * them to the 12-bit DAC. If there is a queue underrun, it turns on
 * the LED briefly but visibly.
 */
void timer_interrupt(void)
{
    static uint8_t led_time;
    uint32_t x;
    if (synth->get_sample(&x) == 0) {
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

int8_t _pitches[] = { 0, 2, 4, 5, 7, 9, 11, 12 };
int8_t *pitches = (int8_t*) &_pitches;

void setup() {
    uint8_t i;
    for (i = 0; i < NUM_VOICES; i++)
        s.add(new NoisyVoice());
    for (i = 0; i < NUM_VOICES; i++)
        s2.add(new SimpleVoice());
    synth = &s;
    analogWriteResolution(12);
    Timer1.initialize((int) (1000000 * DT));
    Timer1.attachInterrupt(timer_interrupt);
    pinMode(0, INPUT_PULLUP);
    pinMode(1, INPUT_PULLUP);
    pinMode(2, INPUT_PULLUP);
    pinMode(3, INPUT_PULLUP);
    pinMode(4, INPUT_PULLUP);
    pinMode(5, INPUT_PULLUP);
    pinMode(6, INPUT_PULLUP);
    pinMode(7, INPUT_PULLUP);
    pinMode(8, INPUT_PULLUP);
    pinMode(9, INPUT_PULLUP);
    pinMode(LED_BUILTIN, OUTPUT);

    for (i = 0; i < NUM_KEYS; i++) {
        keyboard[i] = new Key();
        keyboard[i]->id = i;
        keyboard[i]->pitch = pitches[i];
    }
}

uint8_t read_key(uint32_t id)
{
    if (digitalReadFast(id) == LOW) {
        switch (id) {
            case 8:
                s.quiet();
                s2.quiet();
                cli();
                synth = &s;
                sei();
                return 0;
                break;
            case 9:
                s.quiet();
                s2.quiet();
                cli();
                synth = &s2;
                sei();
                return 0;
                break;
        }
        return 1;
    } else {
        return 0;
    }
}

/**
 * Arduino loop function
 */
void loop(void) {
    int i;
    for (i = 0; i < NUM_KEYS; i++)
        keyboard[i]->check();
    for (i = 0; i < 64; i++)
        synth->compute_sample();
}
