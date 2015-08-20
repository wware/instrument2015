#include <stdlib.h>
#include "common.h"
#include "voice.h"
#include "key.h"
#include "synth.h"

Voice v[NUM_VOICES];
Key *keyboard[NUM_KEYS];

uint8_t read_key(uint32_t id)
{
    uint32_t Y = 0;
    return Y;
}

float small_random()
{
    return -2.0 + 0.01 * (rand() % 400);
}
