#pragma once
#include <stdio.h>

#define sext(val, shift) (val) | (((val) & (1 << (shift))) ? ~((1 << (shift)) - 1) : 0)

typedef u_int8_t  BYTE;
typedef u_int16_t HALF;
typedef u_int32_t WORD;
typedef u_int32_t ADDR;
typedef u_int32_t REG;

typedef enum state {
    QUIT = 0,
    NEXT
} STATE;
