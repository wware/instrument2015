#include <TimerOne.h>

#include "synth.h"
#include "voice.h"

#define NUM_KEYS 38

#define DIAGNOSTICS 0

class ThreadSafeSynth : public Synth
{
    void write_sample(void) {
        cli();
        Synth::write_sample();
        sei();
    }
};

class NoteKey : public Key
{
private:
    uint32_t threshold;

    uint32_t successive_approximate(uint32_t lo, uint32_t hi) {
        if (hi < lo) return successive_approximate(hi, lo);
        if (hi - lo <= 1) return hi;
        uint32_t mid = (lo + hi) >> 1;
        if (!read_n(mid))
            return successive_approximate(lo, mid);
        else
            return successive_approximate(mid, hi);
    }

    bool read_n(uint32_t n) {
        uint32_t chip = id >> 3;
        /*
         * It would be very good to replace all these digitalWrite
         * calls with assembly language. The GPIOs on the Teensy are
         * scrambled relative to the registers, so it's a pain, but doable.
         */
        digitalWrite(10, LOW);
        digitalWrite(4, (id >> 2) & 1);
        digitalWrite(3, (id >> 1) & 1);
        digitalWrite(2, (id >> 0) & 1);
        digitalWrite(5, chip != 0);
        digitalWrite(6, chip != 1);
        digitalWrite(7, chip != 2);
        digitalWrite(8, chip != 3);
        digitalWrite(9, chip != 4);

        volatile uint32_t X = 0, Y = n, portc = 0x400FF080;

        // offset 4 is set output
        // offset 8 is clear output
        // offset 16 is read input
        asm volatile(
            // X = 20;
            "mov %0, #20"                   "\n"

            // while (X > 0) X--;
            "step1:"                        "\n"
            "subs %0, %0, #1"               "\n"
            "bne step1"                     "\n"

            // digitalWrite(10, HIGH);
            "mov %0, #0x10"                 "\n"
            "str %0, [%2, #4]"              "\n"

            // while (Y > 0) Y--;
            "step2:"                        "\n"
            "subs %1, %1, #1"               "\n"
            "bne step2"                     "\n"

            // X = digitalReadFast(11);
            "ldr %0, [%2, #16]"             "\n"
            "ands %0, %0, #0x40"            "\n"

            // digitalWrite(10, LOW);
            "mov %1, #0x10"                 "\n"
            "str %1, [%2, #8]"              "\n"
            : "+r" (X), "+r" (Y), "+r" (portc)
        );
        return !X;
    }

public:
    int fresh_calibrate(void) {
        threshold = 0;
        return calibrate();
    }

    int calibrate(void) {
        const int _max = 250;
        int previous = 0, n;
        for (n = 1; n < _max; n <<=1) {
            if (!read_n(n)) {
                threshold =
                    max(threshold, successive_approximate(previous, n) + 1);
                return threshold;
            }
            previous = n;
        }
        threshold = max(threshold, _max);   // out of luck, probably
        return threshold;
    }

    bool read(void) {
        return read_n(threshold);
    }
};

NoteKey *keyboard[NUM_KEYS];
ThreadSafeSynth s, s2, s3;
ISynth *synth_ary[3];
int which_synth = 0;

class FunctionKey : public NoteKey
{
    void keyup(void) { /* nada */ }
    void keydown(void) {
        int i;
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
            case 36:
                synth_ary[which_synth]->quiet();
                if (keyboard[0]->pitch > -24) {
                    for (i = 0; i < 34; i++) {
                        keyboard[i]->pitch -= 12;
                    }
                }
                break;
            case 37:
                synth_ary[which_synth]->quiet();
                if (keyboard[0]->pitch < 12) {
                    for (i = 0; i < 34; i++) {
                        keyboard[i]->pitch += 12;
                    }
                }
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
    uint8_t i, j;

#if DIAGNOSTICS
    Serial.begin(9600);
    while (!Serial) {
        ; // wait for serial port to connect. Needed for native USB port only
    }
#endif

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
    for (i = 0; i < NUM_SIMPLE_VOICES; i++)
        s.add(new SimpleVoice());
    for (i = 0; i < NUM_NOISY_VOICES; i++)
        s2.add(new NoisyVoice());
    for (i = 0; i < NUM_SQUARE_VOICES; i++)
        s3.add(new TwoSquaresVoice());
    s.quiet();
    use_synth_array(synth_ary, 3);
    use_synth(which_synth);
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
        keyboard[i] = new NoteKey();
        keyboard[i]->id = i;
        keyboard[i]->pitch = i;
    }
    for ( ; i < 34; i++) {
        keyboard[i] = new NoteKey();
        keyboard[i]->id = i;
        keyboard[i]->pitch = i - 5;
    }
    for ( ; i < NUM_KEYS; i++) {
        keyboard[i] = new FunctionKey();
        keyboard[i]->id = i;
    }

    for (i = 0; i < NUM_KEYS; i++) {
        keyboard[i]->fresh_calibrate();
    }
    for (j = 0; j < 3; j++) {
        for (i = 0; i < NUM_KEYS; i++) {
            keyboard[i]->calibrate();
        }
    }
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
    int i, j;

#if DIAGNOSTICS
    i = keyboard[1]->fresh_calibrate();
    for (j = 0; j < i; j++) Serial.print("*");
    Serial.println();
    delay(20);
#else
    for (i = j = 0; i < 8; i++) {
        keyboard[scanned_key]->check();
        scanned_key = (scanned_key + 1) % NUM_KEYS;
    }
    for (i = 0; i < 16; i++) {
        get_synth()->compute_sample();
    }
#endif
}
