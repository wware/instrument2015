#include "common.h"
#include "key.h"

extern uint8_t read_key(uint32_t);

extern ISynth *synth;

void Key::check(void) {
    if (read_key(id) > THRESHOLD) {
        if (state) {
            count = 0;
        } else {
            if (count < KEYDOWN_HYSTERESIS) {
                count++;
                if (count == KEYDOWN_HYSTERESIS) {
                    // this is a keydown event
                    state = 1;
                    count = 0;
                    synth->keydown(pitch);
                }
            }
        }
    } else {
        if (!state) {
            count = 0;
        } else {
            if (count < KEYDOWN_HYSTERESIS) {
                count++;
                if (count == KEYDOWN_HYSTERESIS) {
                    // this is a keyup event
                    state = 0;
                    count = 0;
                    synth->keyup(pitch);
                }
            }
        }
    }
}
