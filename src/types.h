#pragma once
#include <stdio.h>

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

typedef u8 BYTE;
typedef u16 HALF;
typedef u32 WORD;
typedef u32 ADDR;
typedef u32 REG;

union broadcast {
    u64 raw;

    struct broadcast_decoder {
        u32 type: 32;
        u32 info: 32;
    } decoder;
} BROADCAST;

#define STATE u64
#define BROADCAST(stat) BROADCAST.raw = (stat)
#define STAT_QUIT 0
#define STAT_EXIT 1
#define STAT_HALT 2
#define STAT_STEP 3
#define STAT_MEM_EXCEPTION 4
