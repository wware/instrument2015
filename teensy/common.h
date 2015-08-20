/*
We have a 40 kHz or 50 kHz timer interrupt that handles sound
samples. First it writes the previously computed sample to the serial DAC.
Then it computes the sample for the next time.
*/

#ifndef COMMON_H_INCLUDED
#define COMMON_H_INCLUDED 1

#include <stdint.h>

#define SAMPLING_RATE   40000
#define DT (1.0 / SAMPLING_RATE)

#define NUM_KEYS 16   // 17? 34? 40?

#define NUM_VOICES 3

// This is for a 330K resistor.
#define THRESHOLD 10

#define KEYDOWN_HYSTERESIS 10

#define ASSERT(cond)    assertion(cond, #cond, __FILE__, __LINE__)
extern void assertion(int cond, const char *strcond,
					  const char *file, const int line);

#define MIN(x, y)   (((x) < (y)) ? (x) : (y))
#define MAX(x, y)   (((x) > (y)) ? (x) : (y))

#define MULSHIFT32(x, y)  ((((int64_t) x) * y) >> 32)
#define ADDCLIP(x, y)   clip(((int64_t) x) + ((int64_t) y))

extern float small_random();

inline int32_t clip(int64_t x) {
    return MAX(-0x80000000LL, MIN(0x7fffffffLL, x));
}

extern int32_t mult_signed(int32_t x, int32_t y);
extern int32_t mult_unsigned(uint32_t x, uint32_t y);
extern int32_t mult_unsigned_signed(uint32_t x, int32_t y);

#endif     // COMMON_H_INCLUDED
