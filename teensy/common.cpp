#include "common.h"
#ifdef __ASSERT
#include <stdio.h>
#include <stdlib.h>
#endif

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

void assertion(int cond, const char *strcond,
               const char *file, const int line)
{
#ifdef __ASSERT
    if (!cond) {
        fprintf(stderr,
                "%s(%d) ASSERTION FAILED: %s\n",
                file, line, strcond);
        exit(1);
    }
#endif
}
