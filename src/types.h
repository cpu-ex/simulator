#pragma once
#include <stdio.h>

#define sext(val, shift) (val) | (((val) & (1 << (shift))) ? ~((1 << (shift)) - 1) : 0)

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

typedef u8 BYTE;
typedef u16 HALF;
typedef u32 WORD;
typedef u32 ADDR;
typedef u32 REG;

// assuming maximum 8
typedef u32 STATE;
#define STAT_QUIT 0
#define STAT_HALT 1
#define STAT_STEP 2
