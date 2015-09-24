#include <TimerOne.h>

#include "synth.h"
#include "voice.h"

#define THRESHOLD 5
#define NUM_KEYS 36

class ThreadSafeSynth : public Synth
{
    void write_sample(void) {
        cli();
        Synth::write_sample();
        sei();
    }
};

Key *keyboard[NUM_KEYS];
ThreadSafeSynth s, s2, s3;
ISynth *synth_ary[3];
int which_synth = 0;

class FunctionKey : public Key
{
    void keyup(void) { /* nada */ }
    void keydown(void) {
        switch (id) {
            case 34:
                which_synth = (which_synth + 1) % 3;
                synth_ary[which_synth]->quiet();
                cli();
                use_synth(which_synth);
                sei();
                break;
            case 35:
                which_synth = (which_synth + 2) % 3;
                synth_ary[which_synth]->quiet();
                cli();
                use_synth(which_synth);
                sei();
                break;
            default:
                break;
        }
    }
};

/**
 * The timer interrupt takes audio samples from the queue and feeds
 * them to the 12-bit DAC. If there is a queue underrun, it turns on
 * the LED briefly but visibly.
 */
void timer_interrupt(void)
{
    static uint8_t led_time;
    uint32_t x;
    if (get_synth()->get_sample(&x) == 0) {
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

/**
 * Note that there are different numbers of voices assigned for the
 * different types of voice. The more complicated a voice is, the
 * less polyphony is possible. There are two ways to address this.
 *
 * * Lower the sampling rate, which adversely impacts sound quality.
 * * Speed up the code. That means doing a lot of profiling
 *   (best done with a GPIO pin and an oscilloscope in this situation)
 *   and then write tighter C++ code and possibly some assembly language.
 */
void setup() {
    uint8_t i;
    /*
     * The more complicated a voice is, the less polyphony is possible.
     * There are two ways to address this. One is to lower the sampling
     * rate (which adversely impacts sound quality), and speeding up the
     * code. That means doing a lot of profiling (best done with a GPIO
     * pin and an oscilloscope in this situation) possibly followed by
     * tighter C++ code and possibly some assembly language.
     */
#define NUM_NOISY_VOICES  3
#define NUM_SIMPLE_VOICES  6
#define NUM_SQUARE_VOICES  4
    synth_ary[0] = &s;
    synth_ary[1] = &s2;
    synth_ary[2] = &s3;
    for (i = 0; i < NUM_NOISY_VOICES; i++)
        s.add(new NoisyVoice());
    for (i = 0; i < NUM_SIMPLE_VOICES; i++)
        s2.add(new SimpleVoice());
    for (i = 0; i < NUM_SQUARE_VOICES; i++)
        s3.add(new TwoSquaresVoice());
    s.quiet();
    use_synth_array(synth_ary, 3);
    use_synth(which_synth);
    use_read_key(&read_key);
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

    for (i = 0; i < 17; i++) {
        keyboard[i] = new Key();
        keyboard[i]->id = i;
        keyboard[i]->pitch = i;
    }
    for ( ; i < 34; i++) {
        keyboard[i] = new Key();
        keyboard[i]->id = i;
        keyboard[i]->pitch = i - 5;
    }
    for ( ; i < NUM_KEYS; i++) {
        keyboard[i] = new FunctionKey();
        keyboard[i]->id = i;
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
    digitalWrite(5, chip != 0);
    digitalWrite(6, chip != 1);
    digitalWrite(7, chip != 2);
    digitalWrite(8, chip != 3);
    digitalWrite(9, chip != 4);
    Y = 0;
    digitalWrite(10, HIGH);
    while (!digitalReadFast(11)) Y++;
    digitalWrite(10, LOW);
    return Y > THRESHOLD;
}

uint8_t scanned_key = 0;

/**
 * Arduino loop function
 * @todo Read soft pots
 * @todo Read the left-hand keyboard
 * @todo Create Pitch class
 * @todo Map keys to reachable pitches
 */
void loop(void) {
    int i;

    for (i = 0; i < 8; i++) {
        keyboard[scanned_key]->check();
        scanned_key = (scanned_key + 1) % NUM_KEYS;
    }
    for (i = 0; i < 16; i++) {
        get_synth()->compute_sample();
    }
}
