#include "common.h"
#include "key.h"

extern uint8_t read_key(uint32_t);

static uint8_t next_voice_to_assign = 0;

uint32_t Key::check(void) {
    uint32_t i, j, found;
    uint32_t Y = read_key(id);
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

                    // Look for the least recently used voice whose state is zero.
                    for (i = next_voice_to_assign, j = 0, found = 0;
                         j < NUM_VOICES;
                         j++, i = (i + 1) % NUM_VOICES) {
                        if (v[i].adsr.state() == 0) {
                            found = 1;
                            break;
                        }
                    }
                    // If none is found, just grab the least recently used voice.
                    if (!found)
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
