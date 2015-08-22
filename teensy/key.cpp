#include "common.h"
#include "key.h"

extern uint8_t read_key(uint32_t);

extern ISynth *synth;

#define KEYDOWN_HYSTERESIS 10

void Key::check(void) {
    if (read_key(id)) {
        if (state) {
            count = 0;
        } else {
            if (count < KEYDOWN_HYSTERESIS) {
                count++;
                if (count == KEYDOWN_HYSTERESIS) {
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
                    state = 0;
                    count = 0;
                    synth->keyup(pitch);
                }
            }
        }
    }
}
