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

// assuming maximum 8
typedef u32 STATE;
#define STAT_QUIT 0
#define STAT_HALT 1
#define STAT_STEP 2
