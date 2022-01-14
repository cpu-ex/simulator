#include "fpu.h"
#include <math.h>

#define BIT_SET(val, h, l) (val | (((1 << (h - l + 1)) - 1) << l))
#define BIT_SET_ONE(val, n) (val | (1 << n))
#define BIT_GET(val, h, l) ((val >> l) & ((1 << (h - l + 1)) - 1))
#define BIT_GET_ONE(val, n) ((val >> n) & 1)

// fmul
FLOAT_HELPER fmul(const FLOAT_HELPER x1, const FLOAT_HELPER x2) {
    register const u32 h1 = BIT_SET_ONE(BIT_GET(x1.decoder.mantissa, 22, 11), 12); // BIT_SET(m1h, 12, 12)
    register const u32 h2 = BIT_SET_ONE(BIT_GET(x2.decoder.mantissa, 22, 11), 12); // BIT_SET(m2h, 12, 12)
    register const u32 lh = BIT_GET(x1.decoder.mantissa, 10, 0) * h2; // m1l * h2
    register const u32 e3 =  x1.decoder.exp + x2.decoder.exp + 129; // e1 + e2 + 129
    register const u32 m3 = h1 * h2 + ((h1 * BIT_GET(x2.decoder.mantissa, 10, 0)) >> 11) + (lh >> 11) + 2;

    return (FLOAT_HELPER){ .decoder = {
        .mantissa = (BIT_GET_ONE(e3, 8) == 0) ? 0 : ((BIT_GET_ONE(m3, 25) == 1) ? BIT_GET(m3, 24, 2) : BIT_GET(m3, 23, 1)),
        .exp = (BIT_GET_ONE(e3, 8) == 0) ? 0 : ((BIT_GET_ONE(m3, 25) == 1) ? BIT_GET((e3 + 1), 7, 0) : BIT_GET(e3, 7, 0)),
        .sign = x1.decoder.sign ^ x2.decoder.sign
    } };
}

// finv
static f32 finv_table_a[1024];
static f32 finv_table_b[1024];
FLOAT_HELPER finv_m(const u32 mx) {
    return (FLOAT_HELPER){
        .f = (FLOAT_HELPER){ .f = finv_table_b[mx >> 13] }.f - fmul(
            (FLOAT_HELPER){ .f = finv_table_a[mx >> 13] },
            (FLOAT_HELPER){ .decoder = { .mantissa = mx, .exp = 0b01111111, .sign = 0 } }
        ).f
    };
}

// fdiv
FLOAT_HELPER fdiv(const FLOAT_HELPER x1, const FLOAT_HELPER x2) {
    register const u32 e1 = x1.decoder.exp;
    register const u32 e2 = x2.decoder.exp;

    if (e1 == 0) return (FLOAT_HELPER){ .i = 0 };

    register const FLOAT_HELPER c  = finv_m(x2.decoder.mantissa);
    register const FLOAT_HELPER yn = fmul(
        (FLOAT_HELPER){ .decoder = { .mantissa = x1.decoder.mantissa, .exp = 127, .sign = 0 } },
        (FLOAT_HELPER){ .decoder = { .mantissa = c.decoder.mantissa,  .exp = 127, .sign = 0 } }
    );

    register const u32 ey = (c.decoder.mantissa == 0) ? e1 - e2 + yn.decoder.exp : e1 - e2 - 1 + yn.decoder.exp;
    return (FLOAT_HELPER){ .decoder = { .mantissa = yn.decoder.mantissa, .exp = ey, .sign = x1.decoder.sign ^ x2.decoder.sign } };
}

// fsqrt
static f32 fsqrt_table_a[1024];
static f32 fsqrt_table_b[1024];
FLOAT_HELPER fsqrt(const FLOAT_HELPER x) {
    register const u32 sx = x.decoder.sign;
    register const u32 ex = x.decoder.exp;

    if (ex == 0) return (FLOAT_HELPER){ .decoder = { .mantissa = 0, .exp = 0, .sign = sx } };

    register const FLOAT_HELPER offset = { .decoder = {
        .mantissa = x.decoder.mantissa,
        .exp      = ((ex & 1) ? 0b01111110 : 0b10000000) | (ex & 1),
        .sign     = 0
    } };
    register const FLOAT_HELPER c = {
        .f = (FLOAT_HELPER){ .f = fsqrt_table_b[BIT_GET(x.i, 23, 14)] }.f + fmul(
            (FLOAT_HELPER){ .f = fsqrt_table_a[BIT_GET(x.i, 23, 14)] }, offset
        ).f
    }; // fadd
    register const u32 ey = (ex & 1) ? (ex > 127 ? ((ex - 127) >> 1) + 127 : 127 - ((127 - ex) >> 1)) :
                        (ex > 128 ? ((ex - 128) >> 1) + 127 : 127 - ((128 - ex) >> 1));
    return (FLOAT_HELPER){ .decoder = { .mantissa = c.decoder.mantissa, .exp = ey, .sign = sx } };
}

void gen_finv_table(void) {
    union { f32 f; s32 i; } index;
    union { f32 f; s32 i; } x1;
    union { f32 f; s32 i; } x2;
    union { f32 f; s32 i; } x3;
    int mask = (1 << 10) - 1;

    for (int i = 0; i < 1024; ++i) {
        index.i = i;
        x1.i = (127 << 23) + (i << 13);             // left
        x2.i = (127 << 23) + (i << 13) + (1 << 12); // center
        x3.i = (127 << 23) + ((i + 1) << 13);       // right

        f32 atmp  = (1.0 / x1.f - 1.0 / x3.f);
        f32 a     = atmp * 1024.0; 
        f32 amean = (1.0 / x1.f + 1.0 / x3.f) / 2.0;
        f32 b     = (amean + 1.0 / x2.f) / 2.0 + a * x2.f; 
        // b - a*x nearly equal 1/x 

        finv_table_a[index.i & mask] = a;
        finv_table_b[index.i & mask] = b;
    }
}

void gen_fsqrt_table(void) {
    union { f32 f; s32 i; } index;
    union { f32 f; s32 i; } x1;
    union { f32 f; s32 i; } x2;
    union { f32 f; s32 i; } x3;
    int mask = (1 << 10) - 1;
    int mbit = 9;

    for (int i = 0; i < 1024; ++i) {
        int e = 0b01111111; 
        index.i = i;
        x1.i = (e << 23) + (index.i << (23 - mbit));                      // left
        x2.i = (e << 23) + (index.i << (23 - mbit)) + (1 << (22 - mbit)); // center
        x3.i = (e << 23) + ((index.i + 1) << (23 - mbit));                // right

        f32 atmp  = (sqrt(x3.f) - sqrt(x1.f));
        f32 a     = atmp / (x3.f - x1.f);
        f32 amean = (sqrt(x3.f) + sqrt(x1.f)) / 2.0;
        f32 b     = (amean + sqrt(x2.f)) / 2.0 - a * x2.f;
        // a * x + b nearly equal sqrt(x)

        index.i = (i + 512) % 1024; // invert the first bit
        fsqrt_table_a[index.i & mask] = a;
        fsqrt_table_b[index.i & mask] = b;
    }
}

void init_fpu(void) {
    gen_finv_table();
    gen_fsqrt_table();
}