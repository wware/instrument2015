#ifndef KEY_H_INCLUDED
#define KEY_H_INCLUDED 1

#include "voice.h"

class Key {
private:
    uint32_t check_internals(void);

public:
    uint32_t id, state, count;
    float pitch;
    Voice *voice;
    Key() {
        count = state = 0;
        voice = NULL;
    }
    uint32_t check(void);
};

#endif   // KEY_H_INCLUDED
