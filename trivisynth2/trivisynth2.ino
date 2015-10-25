#include <TimerOne.h>

#include "synth.h"
#include "voice.h"
#include "tests.h"

#define DIAGNOSTICS 0
#define KEYBOARD 1
#define NUM_KEYS 24

/** @file */

class ThreadSafeSynth : public Synth
{
    void write_sample(void) {
        cli();
        Synth::write_sample();
        sei();
    }
};

// Port A, Port B, Port D
uint32_t port_settings[3 * NUM_KEYS];

#define PORT_A 0x400FF000
#define PORT_B 0x400FF040
#define PORT_C 0x400FF080
#define PORT_D 0x400FF0C0

static inline void set_port(uint32_t port, uint32_t mask, uint32_t value) {
    // offset 4 is set output
    // offset 8 is clear output
    asm volatile(
        "str %1, [%0, #8]"              "\n"
        "str %2, [%0, #4]"              "\n"
        : "+r" (port), "+r" (mask), "+r" (value)
    );
}

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
        volatile uint32_t X = 0, Y = n, portd = PORT_D;

        set_port(PORT_A, 0x03000, port_settings[3 * id]);
        set_port(PORT_B, 0x30000, port_settings[3 * id + 1]);
        set_port(PORT_D, 0x00085, port_settings[3 * id + 2]);

        // offset 4 is set output
        // offset 8 is clear output
        // offset 16 is read input
        asm volatile(
            // digitalWrite(7, LOW);
            "mov %0, #0x04"                 "\n"
            "str %0, [%2, #8]"              "\n"

            // X = 20;
            "mov %0, #20"                   "\n"

            // while (X > 0) X--;
            "step1:"                        "\n"
            "subs %0, %0, #1"               "\n"
            "bne step1"                     "\n"

            // digitalWrite(7, HIGH);
            "mov %0, #0x04"                 "\n"
            "str %0, [%2, #4]"              "\n"

            // while (Y > 0) Y--;
            "step2:"                        "\n"
            "subs %1, %1, #1"               "\n"
            "bne step2"                     "\n"

            // X = digitalReadFast(8);
            "ldr %0, [%2, #16]"             "\n"
            "ands %0, %0, #0x08"            "\n"

            // digitalWrite(7, LOW);
            "mov %1, #0x04"                 "\n"
            "str %1, [%2, #8]"              "\n"
            : "+r" (X), "+r" (Y), "+r" (portd)
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
int which_synth = 0;
ThreadSafeSynth s, s2, s3;
ISynth *synth_ary[3];

class FunctionKey : public NoteKey
{
    void keyup(void) { /* nada */ }
    void keydown(void) {
        int i;
        switch (id) {
            case 20:
                which_synth = (which_synth + 1) % 3;
                synth_ary[which_synth]->quiet();
                cli();
                use_synth(which_synth);
                sei();
                break;
            case 21:
                which_synth = (which_synth + 2) % 3;
                synth_ary[which_synth]->quiet();
                cli();
                use_synth(which_synth);
                sei();
                break;
            case 22:
                synth_ary[which_synth]->quiet();
                if (keyboard[0]->pitch > -24) {
                    for (i = 0; i < NUM_KEYS - 4; i++) {
                        keyboard[i]->pitch -= 12;
                    }
                }
                break;
            case 23:
                synth_ary[which_synth]->quiet();
                if (keyboard[0]->pitch < 12) {
                    for (i = 0; i < NUM_KEYS - 4; i++) {
                        keyboard[i]->pitch += 12;
                    }
                }
                break;
            default:
                break;
        }
    }
};

extern uint32_t tune[];
uint32_t start_time;

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
    start_time = micros();
    /*
     * The more complicated a voice is, the less polyphony is possible.
     * There are two ways to address this. One is to lower the sampling
     * rate (which adversely impacts sound quality), and speeding up the
     * code. That means doing a lot of profiling (best done with a GPIO
     * pin and an oscilloscope in this situation) possibly followed by
     * tighter C++ code and possibly some assembly language.
     */
#define NUM_NOISY_VOICES  3
#define NUM_SIMPLE_VOICES  12
#define NUM_SQUARE_VOICES  6
    synth_ary[0] = &s;
    synth_ary[1] = &s2;
    synth_ary[2] = &s3;
    for (i = 0; i < NUM_SIMPLE_VOICES; i++)
        s.add(new SimpleVoice());
    for (i = 0; i < NUM_SQUARE_VOICES; i++)
        s2.add(new TwoSquaresVoice());
    for (i = 0; i < NUM_NOISY_VOICES; i++)
        s3.add(new NoisyVoice());
    s.quiet();
    use_synth_array(synth_ary, 3);
    use_synth(0);
    analogWriteResolution(12);
    Timer1.initialize((int) (1000000 * DT));
    Timer1.attachInterrupt(timer_interrupt);
    pinMode(0, OUTPUT);  // B16  -- A
    pinMode(1, OUTPUT);  // B17  -- B
    pinMode(2, OUTPUT);  // D0   -- C
    pinMode(3, OUTPUT);  // A12  -- INH0
    pinMode(4, OUTPUT);  // A13  -- INH1
    pinMode(5, OUTPUT);  // D7   -- INH2
    pinMode(7, OUTPUT);  // D2   -- drive
    pinMode(8, INPUT_PULLUP);  // D3  -- sense
    pinMode(LED_BUILTIN, OUTPUT);

    for (i = 0; i < NUM_KEYS - 4; i++) {
        keyboard[i] = new NoteKey();
        keyboard[i]->id = i;
        keyboard[i]->pitch = i;
    }
    for ( ; i < NUM_KEYS; i++) {
        keyboard[i] = new FunctionKey();
        keyboard[i]->id = i;
        keyboard[i]->pitch = 0;
    }

    for (i = 0; i < NUM_KEYS; i++) {
        uint32_t chip = i >> 3;
        // Port A
        port_settings[3 * i] =
            ((chip != 0) ? 0x1000 : 0) +
            ((chip != 1) ? 0x2000 : 0);
        // Port B
        port_settings[3 * i + 1] = (i & 3) << 16;
        // Port D
        port_settings[3 * i + 2] = ((i & 4) >> 2) +
            ((chip != 2) ? 0x80 : 0);
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

/**
 * Arduino loop function
 */
void loop(void) {
    int i;

#if DIAGNOSTICS
    int j;
    i = keyboard[1]->fresh_calibrate();
    for (j = 0; j < i; j++) Serial.print("*");
    Serial.println();
    delay(20);
#else
#if KEYBOARD
    for (i = 0; i < NUM_KEYS; i++)
        keyboard[i]->check();
#else
    uint32_t msecs = (micros() - start_time) / 1000;
    if (play_tune(tune, msecs)) {
        start_time = micros();
    }
#endif
    for (i = 0; i < 32; i++) {
        get_synth()->compute_sample();
    }
#endif
}
