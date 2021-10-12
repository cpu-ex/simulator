#pragma once
#include <stdio.h>

#define sext(val, shift) (val) | (((val) & (1 << (shift))) ? ~((1 << (shift)) - 1) : 0)

typedef unsigned char u_int8_t;
typedef unsigned short u_int16_t;
typedef unsigned int u_int32_t;

typedef u_int8_t  BYTE;
typedef u_int16_t HALF;
typedef u_int32_t WORD;
typedef u_int32_t ADDR;
typedef u_int32_t REG;

// assuming maximum 8
typedef u_int32_t STATE;
#define STAT_QUIT 0
#define STAT_HALT 1
#define STAT_STEP 2
