#ifndef KEY_H_INCLUDED
#define KEY_H_INCLUDED 1

#include "voice.h"

/**
 * Representation of a key on the keyboard. Handles keyboard scanning
 * and voice assignment.
 */
class Key {
public:
    /**
     * A numerical index of this key, used to address it in the analog mux array.
     */
    uint32_t id;
    /**
     * 1 if the key is touched, 0 otherwise.
     */
    uint32_t state;
    /**
     * This counter is used for hysteresis (debouncing).
     */
    uint32_t count;
    /**
     * This is actually a frequency, not a pitch.
     * @todo Replace this with a pointer to a Pitch instance. (Need a Pitch class.)
     */
    float pitch;
    /**
     * The voice being used to sound this key.
     * @todo Move this pointer to the Pitch class, when it exists.
     */
    Voice *voice;
    Key() {
        count = state = 0;
        voice = NULL;
    }
    /**
     * Capacitively checks to see if this key is being touched.
     * \return RC charge time, in arbitrary units
     */
    uint32_t check(void);
};

#endif   // KEY_H_INCLUDED
