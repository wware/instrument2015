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

#define ASSERT(cond)    assertion(cond, #cond, __FILE__, __LINE__)
extern void assertion(int cond, const char *strcond,
					  const char *file, const int line);

extern float small_random();
extern int32_t mult_signed(int32_t x, int32_t y);
extern int32_t mult_unsigned(uint32_t x, uint32_t y);
extern int32_t mult_unsigned_signed(uint32_t x, int32_t y);

#endif     // COMMON_H_INCLUDED
