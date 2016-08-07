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

#define INH0  2
#define INH1  14
#define INH2  7
#define INH3  8
#define INH4  6

uint32_t inhibits[] = {INH0, INH1, INH2, INH3, INH4};

#define A_SELECT 15
#define B_SELECT 22
#define C_SELECT 23

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
     * An integer, increments for each half-tone in pitch.
     */
    int8_t pitch;
    /**
     * The pitch of the most recent key_down event.
     */
    int8_t last_pitch;

    void keydown(void) {
        usbMIDI.sendNoteOn(pitch, 127, 1);
        last_pitch = pitch;
    }

    void keyup(void) {
        usbMIDI.sendNoteOff(last_pitch, 0, 1);
    }
};

Key *keyboard[NUM_KEYS];


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
}

uint8_t scanned_key = 0;

int chords[4][6] = {
    { 60, 64, 67, 70, 72, 76 },  // C major with B flat
    { 60, 65, 69, 72, 77, 81 },   // F major
    { 62, 65, 67, 71, 74, 77 },   // G major with F
    { 60, 64, 67, 72, 76, 79 }    // C major
};

void loop(void) {
#if _DEMO_MODE
    int i;
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
    keyboard[scanned_key]->check();
    scanned_key = (scanned_key + 1) % NUM_KEYS;
#endif
}
