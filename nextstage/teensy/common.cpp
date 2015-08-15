#include "common.h"

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
