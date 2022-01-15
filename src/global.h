#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// #define LITE_MODE
// #define NO_CACHE

typedef unsigned char      u8;
typedef signed char        s8;
typedef unsigned short     u16;
typedef signed short       s16;
typedef unsigned int       u32;
typedef signed int         s32;
typedef float              f32;
typedef unsigned long long u64;
typedef signed long long   s64;
typedef double             f64;

typedef u8 BYTE;
typedef u16 HALF;
typedef u32 WORD;
typedef u32 ADDR;

union broadcast {
    u64 raw;

    struct broadcast_decoder {
        u64 type: 8;
        s64 info: 48;
    } decoder;
} BROADCAST;

#define STATE u64
#define BROADCAST(stat) BROADCAST.raw = (stat)
#define STAT_QUIT            0
#define STAT_EXIT            1
#define STAT_HALT            2
#define STAT_STEP            3
#define STAT_DUMPING         4
#define STAT_BOOTING         5
#define STAT_MEM_EXCEPTION   6
#define STAT_INSTR_EXCEPTION 7
#define STAT_SHIFT_AMOUNT    8
#define STAT_INFO_MAX        0x7FFFFFFFFFFF

#define max(a, b) ((a) > (b) ? (a) : (b))
#define min(a, b) ((a) < (b) ? (a) : (b))

#define format2big(word)                                                      \
    {                                                                         \
        s32 n = 1;                                                            \
        if (*(char *)&n == 1)                                                 \
        {                                                                     \
            word = ((word & 0xFF00FF00) >> 8) | ((word & 0x00FF00FF) << 8);   \
            word = ((word & 0xFFFF0000) >> 16) | ((word & 0x0000FFFF) << 16); \
        }                                                                     \
    }
