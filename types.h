#pragma once
#include <stdio.h>
#include <stdint.h>

#define sext(val, shift) (val) | (((val) & (1 << (shift))) ? ~((1 << (shift)) - 1) : 0)

typedef uint8_t  BYTE;
typedef uint16_t HALF;
typedef uint32_t WORD;
typedef uint32_t ADDR;
typedef uint32_t REG;

// assuming maximum 8
typedef uint32_t STATE;
#define STAT_QUIT 0
#define STAT_HALT 1
#define STAT_STEP 2
