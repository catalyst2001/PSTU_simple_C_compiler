#pragma once

#define READ_BITS(value, start, end) (((value) >> (start)) & ((1U << ((end) - (start) + 1)) - 1))
#define WRITE_BITS(value, data, start, end) do { \
        int mask = ((1U << ((end) - (start) + 1)) - 1) << (start); \
        (value) = ((value) & ~mask) | (((data) << (start)) & mask); \
    } while (0)