#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
        unsigned long long type: 8;
        signed long long info: 48;
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
