#include "fpu.h"
#include <math.h>

typedef union float_helper {
    u32 i;
    f32 f;

    struct float_decoder {
        u32 mantissa : 23;
        u32 exp : 8;
        u32 sign : 1;
    } __attribute__((packed)) decoder;
} FLOAT_HELPER;

#define BIT_SET(val, h, l) (val | (((1 << (h - l + 1)) - 1) << l))
#define BIT_GET(val, h, l) ((val >> l) & ((1 << (h - l + 1)) - 1))

// fadd
void FADD_EXEC(CORE* const core, const INSTR instr) {
    register const BYTE rd = instr.r.rd;
    register const BYTE rs1 = instr.r.rs1;
    register const BYTE rs2 = instr.r.rs2;
    
    core->fregs[rd] = (FLOAT_HELPER){
        .f = (FLOAT_HELPER){
            .i = core->fregs[rs1]
        }.f + (FLOAT_HELPER){
            .i = core->fregs[rs2]
        }.f
    }.i;
    core->pc += 4;
}

// fsub
void FSUB_EXEC(CORE* const core, const INSTR instr) {
    register const BYTE rd = instr.r.rd;
    register const BYTE rs1 = instr.r.rs1;
    register const BYTE rs2 = instr.r.rs2;
    
    core->fregs[rd] = (FLOAT_HELPER){
        .f = (FLOAT_HELPER){
            .i = core->fregs[rs1]
        }.f - (FLOAT_HELPER){
            .i = core->fregs[rs2]
        }.f
    }.i;
    core->pc += 4;
}

// fmul
FLOAT_HELPER fmul(const FLOAT_HELPER x1 ,const FLOAT_HELPER x2) {
    // register const u32 s1 = x1.decoder.sign;
    // register const u32 s2 = x2.decoder.sign;
    // register const u32 e1 = x1.decoder.exp;
    // register const u32 e2 = x2.decoder.exp;
    // register const u32 m1h = BIT_GET(x1.decoder.mantissa, 22, 11); // x1[22:11]
    // register const u32 m1l = BIT_GET(x1.decoder.mantissa, 10, 0); // x1[10:0]
    // register const u32 m2h = BIT_GET(x2.decoder.mantissa, 22, 11);
    // register const u32 m2l = BIT_GET(x2.decoder.mantissa, 10, 0);

    /* stage 1 */
    // step 2
    register const u32 h1 = BIT_SET(BIT_GET(x1.decoder.mantissa, 22, 11), 12, 12); // BIT_SET(m1h, 12, 12)
    register const u32 h2 = BIT_SET(BIT_GET(x2.decoder.mantissa, 22, 11), 12, 12); // BIT_SET(m2h, 12, 12)
    register const u32 hh = h1 * h2;
    register const u32 hl = h1 * BIT_GET(x2.decoder.mantissa, 10, 0); // h1 * m2l
    register const u32 lh = BIT_GET(x1.decoder.mantissa, 10, 0) * h2; // m1l * h2
    // step 5
    register const u32 s3 = x1.decoder.sign ^ x2.decoder.sign; // s1 ^ s2
    register const u32 e3 =  x1.decoder.exp + x2.decoder.exp + 129; // e1 + e2 + 129

    /* stage 2 */
    // step 3
    register const u32 m3 = hh + (hl >> 11) + (lh >> 11) + 2;
    register const u32 e4 = e3 + 1;

    /* stage 3 */
    register const u32 e5 = (BIT_GET(e3, 8, 8) == 0) ? 0 : ((BIT_GET(m3, 25, 25) == 1) ? BIT_GET(e4, 7, 0) : BIT_GET(e3, 7, 0));
    // step 4
    register const u32 m4 = (BIT_GET(e3, 8, 8) == 0) ? 0 : ((BIT_GET(m3, 25, 25) == 1) ? BIT_GET(m3, 24, 2) : BIT_GET(m3, 23, 1));

    return (FLOAT_HELPER){ .decoder = { .mantissa = m4, .exp = e5, .sign = s3 } };
}

void FMUL_EXEC(CORE* const core, const INSTR instr) {
    register const BYTE rd = instr.r.rd;
    register const BYTE rs1 = instr.r.rs1;
    register const BYTE rs2 = instr.r.rs2;

    core->fregs[rd] = fmul(
        (FLOAT_HELPER){ .i = core->fregs[rs1] },
        (FLOAT_HELPER){ .i = core->fregs[rs2] }
    ).i;
    core->pc += 4;
}

// finv
static f32 finv_table_a[1024];
static f32 finv_table_b[1024];
FLOAT_HELPER finv_m(const u32 mx) {
    // assume mx 23bit
    register const FLOAT_HELPER offset = { .decoder = { .mantissa = mx, .exp = 0b01111111, .sign = 0 } };
    register const FLOAT_HELPER a      = { .f = finv_table_a[mx >> 13] };
    register const FLOAT_HELPER b      = { .f = finv_table_b[mx >> 13] };
    return (FLOAT_HELPER){ .f = b.f - fmul(a, offset).f };
}

// fdiv
FLOAT_HELPER fdiv(const FLOAT_HELPER x1, const FLOAT_HELPER x2) {
    // x1
    register const u32 s1 = x1.decoder.sign;
    register const u32 e1 = x1.decoder.exp;
    register const u32 m1 = x1.decoder.mantissa;
    // x2
    register const u32 s2 = x2.decoder.sign;
    register const u32 e2 = x2.decoder.exp;
    register const u32 m2 = x2.decoder.mantissa;

    if (e1 == 0) {
        return (FLOAT_HELPER){ .i = 0 };
    }

    register const FLOAT_HELPER c   = finv_m(m2);
    register const FLOAT_HELPER x1n = { .decoder = { .mantissa = m1                , .exp = 127, .sign = 0 } };
    register const FLOAT_HELPER x2n = { .decoder = { .mantissa = c.decoder.mantissa, .exp = 127, .sign = 0 } };
    register const FLOAT_HELPER yn  = fmul(x1n, x2n);

    register const u32 ey = (c.decoder.mantissa == 0) ? e1 - e2 + yn.decoder.exp : e1 - e2 - 1 + yn.decoder.exp;
    return (FLOAT_HELPER){ .decoder = { .mantissa = yn.decoder.mantissa, .exp = ey, .sign = (s1 ^ s2) } };
}

void FDIV_EXEC(CORE* const core, const INSTR instr) {
    register const BYTE rd = instr.r.rd;
    register const BYTE rs1 = instr.r.rs1;
    register const BYTE rs2 = instr.r.rs2;

    core->fregs[rd] = fdiv(
        (FLOAT_HELPER){ .i = core->fregs[rs1] },
        (FLOAT_HELPER){ .i = core->fregs[rs2] }
    ).i;
    core->pc += 4;
}

// fsqrt
static f32 fsqrt_table_a[1024];
static f32 fsqrt_table_b[1024];
FLOAT_HELPER fsqrt(const FLOAT_HELPER x) {
    register const u32 sx = x.decoder.sign;
    register const u32 ex = x.decoder.exp;
    register const u32 mx = x.decoder.mantissa;

    if (ex == 0) {
        return (FLOAT_HELPER){ .decoder = { .mantissa = 0, .exp = 0, .sign = sx } };
    }

    register const FLOAT_HELPER a = { .f = fsqrt_table_a[BIT_GET(x.i, 23, 14)] };
    register const FLOAT_HELPER b = { .f = fsqrt_table_b[BIT_GET(x.i, 23, 14)] };
    register const FLOAT_HELPER offset = (ex & 1) ? (FLOAT_HELPER){ .decoder = { .mantissa = mx , .exp = 0b01111110 | (ex & 1), .sign = 0 } }:
                                        (FLOAT_HELPER){ .decoder = { .mantissa = mx , .exp = 0b10000000 | (ex & 1), .sign = 0 } };
    register const FLOAT_HELPER c = { .f = b.f + fmul(a, offset).f }; // fadd
    register const u32 ey = (ex & 1) ? (ex > 127 ? ((ex - 127) >> 1) + 127 : 127 - ((127 - ex) >> 1)):
                        (ex > 128 ? ((ex - 128) >> 1) + 127 : 127 - ((128 - ex) >> 1));
    return (FLOAT_HELPER){ .decoder = { .mantissa = c.decoder.mantissa, .exp = ey, .sign = sx } };
}   

void FSQRT_EXEC(CORE* const core, const INSTR instr) {
    register const BYTE rd = instr.r.rd;
    register const BYTE rs1 = instr.r.rs1;

    core->fregs[rd] = fsqrt((FLOAT_HELPER){ .i = core->fregs[rs1] }).i;
    core->pc += 4;
}

// fcmp
void FCMP_EXEC(CORE* const core, const INSTR instr) {
    register const BYTE rd = instr.r.rd;
    register const BYTE rs1 = instr.r.rs1;
    register const BYTE rs2 = instr.r.rs2;
    register const BYTE funct3 = instr.r.funct3;

    register const FLOAT_HELPER f1 = { .i = core->fregs[rs1] };
    register const FLOAT_HELPER f2 = { .i = core->fregs[rs2] };
    switch (funct3) {
    // feq
    case 0b010: core->regs[rd] = (f1.f == f2.f) ? 1 : 0; break;
    // flt
    case 0b001: core->regs[rd] = (f1.f < f2.f) ? 1 : 0; break;
    // fle
    case 0b000: core->regs[rd] = (f1.f <= f2.f) ? 1 : 0; break;
    // unexpected
    default: BROADCAST(STAT_INSTR_EXCEPTION | ((u64)instr.raw << STAT_SHIFT_AMOUNT)); break;
    }

    core->pc += 4;
}

// fcvt2f convert to float from (unsigned)integer
void FCVT2F_EXEC(CORE* const core, const INSTR instr) {
    register const BYTE rd = instr.r.rd;
    register const BYTE rs1 = instr.r.rs1;
    register const BYTE rs2 = instr.r.rs2;

    register FLOAT_HELPER f;
    switch (rs2) {
    // fcvt.s.w
    case 0b00000: f.f = (f32)((s32)core->regs[rs1]); break;
    // fcvt.s.wu
    case 0b00001: f.f = (f32)((u32)core->regs[rs1]); break;
    // unexpected
    default: BROADCAST(STAT_INSTR_EXCEPTION | ((u64)instr.raw << STAT_SHIFT_AMOUNT)); break;
    }

    core->fregs[rd] = f.i;
    core->pc += 4;
}

// fcvt2i convert to (unsigned)integer from float
void FCVT2I_EXEC(CORE* const core, const INSTR instr) {
    register const BYTE rd = instr.r.rd;
    register const BYTE rs1 = instr.r.rs1;
    register const BYTE rs2 = instr.r.rs2;

    register const FLOAT_HELPER f = { .i = core->fregs[rs1] };
    register u32 i;
    switch (rs2) {
    // fcvt.w.s
    case 0b00000: i = (f.f < 0.0) ? ((s32)(f.f - (f32)((s32)(f.f - 1.0)) + 0.5) + (s32)(f.f - 1.0)) : ((s32)(f.f + 0.5)); break;
    // fcvt.wu.s
    case 0b00001: i = (u32)(f.f + 0.5); break;
    // unexpected
    default: BROADCAST(STAT_INSTR_EXCEPTION | ((u64)instr.raw << STAT_SHIFT_AMOUNT)); break;
    }

    core->regs[rd] = i;
    core->pc += 4;
}

// fsgnj float sign inject
void FSGNJ_EXEC(CORE* const core, const INSTR instr) {
    register const BYTE rd = instr.r.rd;
    register const BYTE rs1 = instr.r.rs1;
    register const BYTE rs2 = instr.r.rs2;
    register const BYTE funct3 = instr.r.funct3;

    register FLOAT_HELPER f1 = { .i = core->fregs[rs1] };
    register const FLOAT_HELPER f2 = { .i = core->fregs[rs2] };

    switch (funct3) {
    // fsgnj
    case 0b000: f1.decoder.sign = f2.decoder.sign; break;
    // fsgnjn
    case 0b001: f1.decoder.sign = ~f2.decoder.sign; break;
    // fsgnjx
    case 0b010: f1.decoder.sign = f1.decoder.sign ^ f2.decoder.sign; break;
    // unexpected
    default: BROADCAST(STAT_INSTR_EXCEPTION | ((u64)instr.raw << STAT_SHIFT_AMOUNT)); break;
    }

    core->fregs[rd] = f1.i;
    core->pc += 4;
}

void gen_finv_table(void) {
    union { f32 f; s32 i; } index;
    union { f32 f; s32 i; } x1;
    union { f32 f; s32 i; } x2;
    union { f32 f; s32 i; } x3;
    int mask = (1 << 10) - 1;

    for (int i = 0; i < 1024; i++) {
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

    for (int i = 0; i < 1024; i++) {
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