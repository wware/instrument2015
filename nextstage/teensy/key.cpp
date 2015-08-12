#include "common.h"
#include "key.h"

static uint8_t next_voice_to_assign = 0;

uint32_t Key::check_internals(void) {
    uint32_t Y = 0;
#if __ARDUINO
    uint32_t i, j = id, chip;
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
    // while (!digitalRead(11)) Y++;
    while (!digitalReadFast(11)) Y++;
    digitalWrite(10, LOW);
#endif
    return Y;
}

uint32_t Key::check(void) {
    uint32_t i, j = id;
    uint32_t Y = check_internals();
    if (Y > THRESHOLD) {
        if (state) {
            count = 0;
        } else {
            if (count < KEYDOWN_COUNT) {
                count++;
                if (count == KEYDOWN_COUNT) {
                    // this is a keydown event
                    state = 1;
                    count = 0;

                    i = next_voice_to_assign;
                    next_voice_to_assign = (i + 1) % NUM_VOICES;

                    // does some other key already have this voice? If so, remove
                    // the voice from that other key
                    for (j = 0; j < NUM_KEYS; j++)
                        if (j != id && keyboard[j]->voice == &v[i]) {
                              keyboard[j]->voice = NULL;
                        }

                    voice = &v[i];
                    voice->setfreq(pitch);
                    voice->keydown(1);
                }
            }
        }
    } else {
        if (!state) {
            count = 0;
        } else {
            if (count < KEYDOWN_COUNT) {
                count++;
                if (count == KEYDOWN_COUNT) {
                    // this is a keyup event
                    state = 0;
                    count = 0;
                    if (voice != NULL) {
                        voice->keydown(0);
                        voice = NULL;
                    }
                }
            }
        }
    }
    return Y;
}
