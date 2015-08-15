/*
We have a 40 kHz or 50 kHz timer interrupt that handles sound
samples. First it writes the previously computed sample to the serial DAC.
Then it computes the sample for the next time.
*/

#ifndef COMMON_H_INCLUDED
#define COMMON_H_INCLUDED 1

#ifndef __STDINT
#include <stdint.h>
#define __STDINT 1
#endif

#ifndef __ARDUINO
#define __ARDUINO 1
#endif

#ifndef ASSERT
#define ASSERT(cond)    ((void*) 0)
#endif

#ifndef NULL
#define NULL   0
#endif

#define SAMPLING_RATE   40000
#define DT (1.0 / SAMPLING_RATE)

#define ADSR_BITS   30
#define ADSR_MAX (1 << ADSR_BITS)

#define NUM_KEYS 16   // 17? 34? 40?
#define NUM_VOICE_BITS 3
#define NUM_VOICES (1 << NUM_VOICE_BITS)


/* 1 / (1 - 1/e), because exponential */
#define BIGGER 1.5819767

// This is for a 330K resistor.
#define THRESHOLD 12

#define KEYDOWN_COUNT 12

class Key;
extern Key *keyboard[NUM_KEYS];
class Voice;
extern Voice v[NUM_VOICES];

#endif     // COMMON_H_INCLUDED
