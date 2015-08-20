#ifndef KEY_H_INCLUDED
#define KEY_H_INCLUDED 1

#include "common.h"
#include "synth.h"

/**
 * Representation of a key on the keyboard. Handles keyboard scanning
 * and issuing keydown/keyup events to an ISynth instance.
 */
class Key {
public:
    /**
     * A numerical index of this key, used to identify it for keyboard scanning.
     */
    uint32_t id;
    /**
     * 1 if the key is pressed/touched, 0 otherwise.
     */
    uint32_t state;
    /**
     * This counter is used for hysteresis (debouncing).
     */
    uint32_t count;
    /**
     * An integer, increments for each half-tone in pitch.
     */
    int8_t pitch;
    /**
     * The voice being used to sound this key.
     */
    IVoice *voice;

    Key() {
        count = state = 0;
        voice = NULL;
    }
    /**
     * Checks to see if this key is being pressed/touched.
     */
    void check(void);
};

#endif   // KEY_H_INCLUDED
