#include "common.h"
#if !__ARDUINO
#include <stdio.h>
#include <stdlib.h>
#endif

int32_t clip(int64_t x) {
    if (x > 0x7fffffffLL) x = 0x7fffffffLL;
    if (x < -0x80000000LL) x = -0x80000000LL;
    return (int32_t) x;
}

int32_t mult_signed(int32_t x, int32_t y)
{
    return (((int64_t) x) * y) >> 32;
}

int32_t mult_unsigned(uint32_t x, uint32_t y)
{
    return (((uint64_t) x) * y) >> 32;
}

int32_t mult_unsigned_signed(uint32_t x, int32_t y)
{
    return (((uint64_t) x) * y) >> 32;
}

void assertion(int cond, const char *strcond)
{
#if !__ARDUINO
    if (!cond) {
        fprintf(stderr, "ASSERTION FAILED: %s\n", strcond);
        exit(1);
    }
#endif
}
