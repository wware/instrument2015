/**
 * Arduino/Teensy settings:
 *   Teensy 3.2 / 3.1
 *   USB Type: MIDI  (nothing additional)
 *   CPU Speed: 72 MHz
 */

extern unsigned char chord_table[];

#include "keys.h"
#define _DEMO_MODE 0
#define _HACK 0

// Something is wonky about the eight high-numbered key connections
// on this PC board, probably some kind of assembly error, but the
// lower 32 are working OK.
#define NUM_KEYS 32

#define DIAGNOSTICS 0

#define PIANO            0
#define CELESTA         10
#define TUBULAR_BELLS   14
#define CHURCH_ORGAN    19
#define NYLON_GUITAR    24
#define CELLO           42
#define ORCHESTRAL_HARP 46
#define CHOIR_AAHS      52
#define TRUMPET         56
#define SYNTH_BRASS     62
#define ALTO_SAX        65
#define FLUTE           73

uint8_t voices[] = {
    PIANO, CELESTA, CHURCH_ORGAN, NYLON_GUITAR,
    CELLO, CHOIR_AAHS, TRUMPET, FLUTE
};

#define INH0  2
#define INH1  14
#define INH2  7
#define INH3  8
#define INH4  6

uint32_t inhibits[] = {INH0, INH1, INH2, INH3, INH4};

#define A_SELECT 15
#define B_SELECT 22
#define C_SELECT 23

/**
 * A pointer into chord_table, where we find the pitches for the sevn
 * strings.
 */
uint8_t *chord_pointer = &chord_table[7 * 6];

/**
 * If we want to transpose the chord up or down by some number of octaves,
 * do that here. This should be a multiple of 12.
 */
int8_t chord_transpose = 0;

// Port A, Port C, Port D
uint32_t port_settings[2 * NUM_KEYS];

#define PORT_A 0x400FF000
#define PORT_C 0x400FF080
#define PORT_D 0x400FF0C0

static inline uint32_t read_key(uint32_t X, uint32_t Y, uint32_t portc) {
    // offset 4 is set output
    // offset 8 is clear output
    // offset 16 is read input
    asm volatile(
        // digitalWrite(9, LOW);
        "mov %0, #0x08"                 "\n"
        "str %0, [%2, #8]"              "\n"

        // X = 20;
        "mov %0, #20"                   "\n"

        // while (X > 0) X--;
        "1:"                            "\n"
        "subs %0, %0, #1"               "\n"
        "bne 1b"                        "\n"

        // digitalWrite(9, HIGH);
        "mov %0, #0x08"                 "\n"
        "str %0, [%2, #4]"              "\n"

        // while (Y > 0) Y--;
        "2:"                            "\n"
        "subs %1, %1, #1"               "\n"
        "bne 2b"                        "\n"

        // X = digitalReadFast(10);
        "ldr %0, [%2, #16]"             "\n"
        "ands %0, %0, #0x10"            "\n"

        // digitalWrite(9, LOW);
        "mov %1, #0x08"                 "\n"
        "str %1, [%2, #8]"              "\n"
        : "+r" (X), "+r" (Y), "+r" (portc)
    );
    return !X;
}

void flash_int(int n)
{
    int i = n / 5;
    n -= 5 * i;
    while (i--) {
        digitalWrite(LED_BUILTIN, HIGH);
        delayMicroseconds(1000 * 1000);
        digitalWrite(LED_BUILTIN, LOW);
        delayMicroseconds(200 * 1000);
    }
    while (n--) {
        digitalWrite(LED_BUILTIN, HIGH);
        delayMicroseconds(200 * 1000);
        digitalWrite(LED_BUILTIN, LOW);
        delayMicroseconds(200 * 1000);
    }
}


int h = 0;

class Key : public BaseKey
{
public:
    Key(uint32_t _id) : BaseKey(_id) {}

    bool read_n(uint32_t n) {
        digitalWrite(A_SELECT, (id & 1) ? HIGH : LOW);
        digitalWrite(B_SELECT, (id & 2) ? HIGH : LOW);
        digitalWrite(C_SELECT, (id & 4) ? HIGH : LOW);
        unsigned int i;
        for (i = 0; i < 5; i++) {
            if (i == (id >> 3))
                digitalWrite(inhibits[i], LOW);
            else
                digitalWrite(inhibits[i], HIGH);
        }
        return read_key(0, n, PORT_C);
    }
};

class HackKey : public Key
{
public:
    HackKey(uint32_t _id) : Key(_id) {}

    void keydown(void) {
        flash_int(id + 1);
    }
};

class StringKey : public Key
{
public:
    StringKey(uint32_t _id) : Key(_id) {}

    /**
     * The pitch of the most recent key_down event.
     */
    int8_t last_pitch;

    void keydown(void) {
        int8_t pitch = chord_pointer[id];
        usbMIDI.sendNoteOn(pitch, 127, 1);
        last_pitch = pitch;
    }

    void keyup(void) {
        usbMIDI.sendNoteOff(last_pitch, 0, 1);
    }
};

Key *keyboard[NUM_KEYS];
static uint8_t program = PIANO;

void setup() {
    uint8_t j;

    pinMode(LED_BUILTIN, OUTPUT);

    uint8_t i;

    usbMIDI.sendProgramChange(CHOIR_AAHS, 1);

    pinMode(10, INPUT_PULLUP);
    pinMode(9, OUTPUT);
    digitalWrite(9, LOW);
    pinMode(2, OUTPUT);
    pinMode(6, OUTPUT);
    pinMode(7, OUTPUT);
    pinMode(8, OUTPUT);
    pinMode(14, OUTPUT);
    pinMode(15, OUTPUT);
    pinMode(22, OUTPUT);
    pinMode(23, OUTPUT);

#if _HACK
    for (i = 0; i < NUM_KEYS; i++) {
        keyboard[i] = new HackKey(i);
    }
#else
    for (i = 0; i < NUM_KEYS; i++) {
        if (i < 7)
            keyboard[i] = new StringKey(i);
        else
            keyboard[i] = new Key(i);
    }
#endif

    for (i = 0; i < NUM_KEYS; i++) {
        keyboard[i]->fresh_calibrate();
    }

    for (j = 0; j < 3; j++) {
        for (i = 0; i < NUM_KEYS; i++) {
            keyboard[i]->calibrate();
        }
    }

    for (i = 0; i < 5; i++) {
        usbMIDI.sendProgramChange(program, 1);
        delayMicroseconds(100 * 1000);
    }
}

int chords[4][6] = {
    { 60, 64, 67, 70, 72, 76 },  // C major with B flat
    { 60, 65, 69, 72, 77, 81 },   // F major
    { 62, 65, 67, 71, 74, 77 },   // G major with F
    { 60, 64, 67, 72, 76, 79 }    // C major
};

void loop(void) {
    int i;
#if _DEMO_MODE
    static int j = 0;

    delayMicroseconds(500000);

    switch (j) {
    case 0:
        usbMIDI.sendProgramChange(NYLON_GUITAR, 1);
        break;
    case 1:
        usbMIDI.sendProgramChange(CELESTA, 1);
        break;
    case 2:
        usbMIDI.sendProgramChange(TRUMPET, 1);
        break;
    case 3:
        usbMIDI.sendProgramChange(FLUTE, 1);
        break;
    }

    for (i = 0; i < 6; i++) {
        usbMIDI.sendNoteOn(chords[j][i] - 12, 127, 1);
        delayMicroseconds(100000);
    }
    delayMicroseconds(50000);
    for (i = 0; i < 6; i++) {
        usbMIDI.sendNoteOff(chords[j][i] - 12, 0, 1);
        delayMicroseconds(100000);
    }

    j = (j + 1) % 4;
    if (j == 0) delayMicroseconds(500000);
#else
    uint8_t *new_chord_pointer;

    /**
     * An index into the circle of fifths indicating the base note of
     * the triad for this chord. 0=G-flat, 1=D-flat, 2=E-flat... 6=C, 7=G
     * up to 11=B.
     */
    /*
    static uint8_t chord_base;
    int8_t new_chord_base;
    static uint8_t chord_base_key_pressed = 0;
    int8_t new_chord_base_key_pressed;
     */

    static KeyState chord_base_state;
    static KeyState chord_modifier_state;

    /**
     * The three modifier strings determine a choice of eight posssible
     * chord types.
     */
    static uint8_t add_seventh = 0;

    /*
    static uint8_t chord_modifier;
    uint8_t new_chord_modifier;
    static uint8_t chord_modifier_key_pressed = 0;
    int8_t new_chord_modifier_key_pressed;
     */

    for (i = 7; i < NUM_KEYS; i++)
        keyboard[i]->check();

    // new_chord_base_key_pressed = 0;
    chord_base.unpress();
    for (i = 7; i < 19; i++)
        if (keyboard[i]->state) {
            new_chord_base = i - 7;
            chord_base.press();
            // new_chord_base_key_pressed = 1;
            break;
        }

    chord_modifier.set_new_value(
        (keyboard[19]->state ? 4 : 0) +
        (keyboard[20]->state ? 2 : 0)
    );

    new_chord_modifier =
        (keyboard[19]->state ? 4 : 0) +
        (keyboard[20]->state ? 2 : 0);
    new_chord_modifier_key_pressed = !!new_chord_modifier;

    if (new_chord_modifier_key_pressed) {
        if (keyboard[21]->state) {
            add_seventh = 1;
        }
        else if (!chord_modifier_key_pressed) {
            add_seventh = 0;
        }
        chord_modifier = new_chord_modifier;
    }
    chord_modifier_key_pressed = new_chord_modifier_key_pressed;

    if (new_chord_base_key_pressed)
        chord_base = new_chord_base;
    chord_base_key_pressed = new_chord_base_key_pressed;

    if (chord_base_key_pressed || chord_modifier_key_pressed) {
        uint8_t x = chord_modifier + (add_seventh ? 1 : 0);
        new_chord_pointer = &chord_table[7 * (12 * x + chord_base)];
        if (new_chord_pointer != chord_pointer) {
            chord_pointer = new_chord_pointer;
            for (i = 0; i < 7; i++) {
                if (keyboard[i]->state &&
                    chord_pointer[i] != ((StringKey*)keyboard[i])->last_pitch) {
                    keyboard[i]->keyup();
                    keyboard[i]->keydown();
                }
            }
        }
        chord_pointer = new_chord_pointer;
    }

    // select the instrument
    for (i = 22; i < 30; i++)
        if (keyboard[i]->state) {
            i = voices[i - 22];
            if (i != program) {
                usbMIDI.sendProgramChange(i, 1);
                program = i;
            }
            break;
        }

    for (i = 0; i < 7; i++)
        keyboard[i]->check();
#endif
}
