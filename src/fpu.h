#pragma once
#include "global.h"

typedef union float_helper {
    u32 i;
    f32 f;

    struct float_decoder {
        u32 mantissa : 23;
        u32 exp : 8;
        u32 sign : 1;
    } __attribute__((packed)) decoder;
} FLOAT_HELPER;

const FLOAT_HELPER fmul(const FLOAT_HELPER, const FLOAT_HELPER);
const FLOAT_HELPER fdiv(const FLOAT_HELPER, const FLOAT_HELPER);
const FLOAT_HELPER fsqrt(const FLOAT_HELPER);
void init_fpu(void);
