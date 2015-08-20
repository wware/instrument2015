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

#ifndef NULL
#define NULL   0
#endif

#define USE_FILTER 1

#define SAMPLING_RATE   30000
#define DT (1.0 / SAMPLING_RATE)

#define NUM_KEYS 16   // 17? 34? 40?

#if USE_FILTER
#define NUM_VOICES 4
#else
#define NUM_VOICES 8
#endif

// This is for a 330K resistor.
#define THRESHOLD 12

#define KEYDOWN_HYSTERESIS 10

#define ASSERT(cond)    assertion(cond, #cond, __FILE__, __LINE__)
extern void assertion(int cond, const char *strcond,
					  const char *file, const int line);

#define MIN(x, y)   (((x) < (y)) ? (x) : (y))
#define MAX(x, y)   (((x) > (y)) ? (x) : (y))

#define MULDIV32(x, y)  ((((int64_t) x) * y) >> 32)
#define ADDCLIP(x, y)   clip(((int64_t) x) + ((int64_t) y))

extern float small_random();

class Key;
extern Key *keyboard[NUM_KEYS];
class Voice;
extern Voice v[NUM_VOICES];

inline int32_t clip(int64_t x) {
    return MAX(-0x80000000LL, MIN(0x7fffffffLL, x));
}

extern int32_t mult_signed(int32_t x, int32_t y);
extern int32_t mult_unsigned(uint32_t x, uint32_t y);
extern int32_t mult_unsigned_signed(uint32_t x, int32_t y);

#endif     // COMMON_H_INCLUDED
