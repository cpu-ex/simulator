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

// f-load
void FLW_EXEC(CORE* core, INSTR instr) {
    BYTE rd = instr.i.rd;
    WORD imm = instr.i.imm;
    BYTE rs1 = instr.i.rs1;
    BYTE funct3 = instr.i.funct3;
    switch (funct3) {
    // flw
    case 0b010: core->fregs[rd] = core->load_data(core, core->regs[rs1] + sext(imm, 11), 2, 0); break;
    // unexpected
    default: BROADCAST(STAT_INSTR_EXCEPTION | ((u64)instr.raw << STAT_SHIFT_AMOUNT)); break;
    }
    core->pc += 4;
    // stall check
    core->stall_counter += isLwStall(rd, core->load_instr(core, core->pc)) ? 1 : 0;
}

// f-store
void FSW_EXEC(CORE* core, INSTR instr) {
    WORD imm = instr.s.imm11_5 << 5 |
                instr.s.imm4_0;
    BYTE rs1 = instr.s.rs1;
    BYTE rs2 = instr.s.rs2;
    BYTE funct3 = instr.s.funct3;
    switch (funct3) {
    // fsw
    case 0b010: core->store_data(core, core->regs[rs1] + sext(imm, 11), core->fregs[rs2], 2); break;
    // unexpected
    default: BROADCAST(STAT_INSTR_EXCEPTION | ((u64)instr.raw << STAT_SHIFT_AMOUNT)); break;
    }
    core->pc += 4;
}

// f-mv to integer from float (loose check)
void FMV2I_EXEC(CORE* core, INSTR instr) {
    BYTE rd = instr.r.rd;
    BYTE rs1 = instr.r.rs1;

    core->regs[rd] = core->fregs[rs1];
    core->pc += 4;
}

// f-mv to float from integer (loose check)
void FMV2F_EXEC(CORE* core, INSTR instr) {
    BYTE rd = instr.r.rd;
    BYTE rs1 = instr.r.rs1;

    core->fregs[rd] = core->regs[rs1];
    core->pc += 4;
}

// fadd
void FADD_EXEC(CORE* core, INSTR instr) {
    BYTE rd = instr.r.rd;
    BYTE rs1 = instr.r.rs1;
    BYTE rs2 = instr.r.rs2;
    
    FLOAT_HELPER f1 = { .i = core->fregs[rs1] };
    FLOAT_HELPER f2 = { .i = core->fregs[rs2] };
    FLOAT_HELPER val = { .f = f1.f + f2.f };

    core->fregs[rd] = val.i;
    core->pc += 4;
}

// fsub
void FSUB_EXEC(CORE* core, INSTR instr) {
    BYTE rd = instr.r.rd;
    BYTE rs1 = instr.r.rs1;
    BYTE rs2 = instr.r.rs2;
    
    FLOAT_HELPER f1 = { .i = core->fregs[rs1] };
    FLOAT_HELPER f2 = { .i = core->fregs[rs2] };
    FLOAT_HELPER val = { .f = f1.f - f2.f };

    core->fregs[rd] = val.i;
    core->pc += 4;
}

// fmul
FLOAT_HELPER fmul(FLOAT_HELPER x1,FLOAT_HELPER x2) {
    u32 s1 = x1.decoder.sign;
    u32 s2 = x2.decoder.sign;
    u32 e1 = x1.decoder.exp;
    u32 e2 = x2.decoder.exp;
    u32 m1h = BIT_GET(x1.decoder.mantissa, 22, 11); // x1[22:11]
    u32 m1l = BIT_GET(x1.decoder.mantissa, 10, 0); // x1[10:0]
    u32 m2h = BIT_GET(x2.decoder.mantissa, 22, 11);
    u32 m2l = BIT_GET(x2.decoder.mantissa, 10, 0);

    /* stage 1 */
    // step 2
    u32 h1 = BIT_SET(m1h, 12, 12);
    u32 h2 = BIT_SET(m2h, 12, 12);
    u32 hh = h1 * h2;
    u32 hl = h1 * m2l;
    u32 lh = m1l * h2;
    // step 5
    u32 s3 = s1 ^ s2;
    u32 e3 = e1 + e2 + 129;

    /* stage 2 */
    // step 3
    u32 m3 = hh + (hl >> 11) + (lh >> 11) + 2;
    u32 e4 = e3 + 1;

    /* stage 3 */
    u32 e5 = (BIT_GET(e3, 8, 8) == 0) ? 0 : ((BIT_GET(m3, 25, 25) == 1) ? BIT_GET(e4, 7, 0) : BIT_GET(e3, 7, 0));
    // step 4
    u32 m4 = (BIT_GET(e3, 8, 8) == 0) ? 0 : ((BIT_GET(m3, 25, 25) == 1) ? BIT_GET(m3, 24, 2) : BIT_GET(m3, 23, 1));

    FLOAT_HELPER val = { .decoder = { .mantissa = m4, .exp = e5, .sign = s3 } };
    return val;
}

void FMUL_EXEC(CORE* core, INSTR instr) {
    BYTE rd = instr.r.rd;
    BYTE rs1 = instr.r.rs1;
    BYTE rs2 = instr.r.rs2;

    FLOAT_HELPER x1 = { .i = core->fregs[rs1] };
    FLOAT_HELPER x2 = { .i = core->fregs[rs2] };
    FLOAT_HELPER val = fmul(x1, x2);

    core->fregs[rd] = val.i;
    core->pc += 4;
}

// finv
f32 finv_table_a[1024];
f32 finv_table_b[1024];
FLOAT_HELPER finv_m(u32 mx){
    // assume mx 23bit
    FLOAT_HELPER offset = { .decoder = { .mantissa = mx, .exp = 0b01111111, .sign = 0 } };
    FLOAT_HELPER a      = { .f = finv_table_a[mx >> 13] };
    FLOAT_HELPER b      = { .f = finv_table_b[mx >> 13] };
    FLOAT_HELPER ax     = fmul(a, offset);
    FLOAT_HELPER c      = { .f = b.f - ax.f }; // fsub
    return c;
}

// fdiv
FLOAT_HELPER fdiv(FLOAT_HELPER x1, FLOAT_HELPER x2) {
    // x1
    u32 s1 = x1.decoder.sign;
    u32 e1 = x1.decoder.exp;
    u32 m1 = x1.decoder.mantissa;
    // x2
    u32 s2 = x2.decoder.sign;
    u32 e2 = x2.decoder.exp;
    u32 m2 = x2.decoder.mantissa;

    if (e1 == 0) {
        return (FLOAT_HELPER){ .i = 0 };
    }

    FLOAT_HELPER c   = finv_m(m2);
    FLOAT_HELPER x1n = { .decoder = { .mantissa = m1                , .exp = 127, .sign = 0 } };
    FLOAT_HELPER x2n = { .decoder = { .mantissa = c.decoder.mantissa, .exp = 127, .sign = 0 } };
    FLOAT_HELPER yn  = fmul(x1n, x2n);

    u32 ey = (c.decoder.mantissa == 0) ? e1 - e2 + yn.decoder.exp : e1 - e2 - 1 + yn.decoder.exp;
    FLOAT_HELPER y = { .decoder = { .mantissa = yn.decoder.mantissa, .exp = ey, .sign = (s1 ^ s2) } };
    return y;
}

void FDIV_EXEC(CORE* core, INSTR instr) {
    BYTE rd = instr.r.rd;
    BYTE rs1 = instr.r.rs1;
    BYTE rs2 = instr.r.rs2;

    FLOAT_HELPER f1 = { .i = core->fregs[rs1] };
    FLOAT_HELPER f2 = { .i = core->fregs[rs2] };
    FLOAT_HELPER val = fdiv(f1, f2);

    core->fregs[rd] = val.i;
    core->pc += 4;
}

// fsqrt
f32 fsqrt_table_a[1024];
f32 fsqrt_table_b[1024];
FLOAT_HELPER fsqrt(FLOAT_HELPER x) {
    u32 sx = x.decoder.sign;
    u32 ex = x.decoder.exp;
    u32 mx = x.decoder.mantissa;

    if (ex == 0) {
        return (FLOAT_HELPER){ .decoder = { .mantissa = 0, .exp = 0, .sign = sx } };
    }

    FLOAT_HELPER a = { .f = fsqrt_table_a[BIT_GET(x.i, 23, 14)] };
    FLOAT_HELPER b = { .f = fsqrt_table_b[BIT_GET(x.i, 23, 14)] };
    FLOAT_HELPER offset = (ex & 1) ? (FLOAT_HELPER){ .decoder = { .mantissa = mx , .exp = 0b01111110 | (ex & 1), .sign = 0 } }:
                                        (FLOAT_HELPER){ .decoder = { .mantissa = mx , .exp = 0b10000000 | (ex & 1), .sign = 0 } };
    FLOAT_HELPER ax = fmul(a, offset);
    FLOAT_HELPER c = { .f = b.f + ax.f }; // fadd
    u32 ey = (ex & 1) ? (ex > 127 ? ((ex - 127) >> 1) + 127 : 127 - ((127 - ex) >> 1)):
                        (ex > 128 ? ((ex - 128) >> 1) + 127 : 127 - ((128 - ex) >> 1));
    FLOAT_HELPER y = { .decoder = { .mantissa = c.decoder.mantissa, .exp = ey, .sign = sx } };
    return y;
}   

void FSQRT_EXEC(CORE* core, INSTR instr) {
    BYTE rd = instr.r.rd;
    BYTE rs1 = instr.r.rs1;

    FLOAT_HELPER f1 = { .i = core->fregs[rs1] };
    FLOAT_HELPER val = fsqrt(f1);

    core->fregs[rd] = val.i;
    core->pc += 4;
}

// fcmp
void FCMP_EXEC(CORE* core, INSTR instr) {
    BYTE rd = instr.r.rd;
    BYTE rs1 = instr.r.rs1;
    BYTE rs2 = instr.r.rs2;
    BYTE funct3 = instr.r.funct3;

    FLOAT_HELPER f1 = { .i = core->fregs[rs1] };
    FLOAT_HELPER f2 = { .i = core->fregs[rs2] };
    u32 val;
    switch (funct3) {
    // feq
    case 0b010: val = (f1.f == f2.f) ? 1 : 0; break;
    // flt
    case 0b001: val = (f1.f < f2.f) ? 1 : 0; break;
    // fle
    case 0b000: val = (f1.f <= f2.f) ? 1 : 0; break;
    // unexpected
    default: BROADCAST(STAT_INSTR_EXCEPTION | ((u64)instr.raw << STAT_SHIFT_AMOUNT)); break;
    }

    core->regs[rd] = val;
    core->pc += 4;
}

// fcvt2f convert to float from (unsigned)integer
void FCVT2F_EXEC(CORE* core, INSTR instr) {
    BYTE rd = instr.r.rd;
    BYTE rs1 = instr.r.rs1;
    BYTE rs2 = instr.r.rs2;

    FLOAT_HELPER f;
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
void FCVT2I_EXEC(CORE* core, INSTR instr) {
    BYTE rd = instr.r.rd;
    BYTE rs1 = instr.r.rs1;
    BYTE rs2 = instr.r.rs2;

    FLOAT_HELPER f = {.i = core->fregs[rs1]};
    u32 i;
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
void FSGNJ_EXEC(CORE* core, INSTR instr) {
    BYTE rd = instr.r.rd;
    BYTE rs1 = instr.r.rs1;
    BYTE rs2 = instr.r.rs2;
    BYTE funct3 = instr.r.funct3;

    FLOAT_HELPER f1 = { .i = core->fregs[rs1] };
    FLOAT_HELPER f2 = { .i = core->fregs[rs2] };
    FLOAT_HELPER val = { .decoder = { .mantissa = f2.decoder.mantissa, .exp = f2.decoder.exp } };
    switch (funct3) {
    // fsgnj
    case 0b000: val.decoder.sign = f2.decoder.sign; break;
    // fsgnjn
    case 0b001: val.decoder.sign = ~f2.decoder.sign; break;
    // fsgnjx
    case 0b010: val.decoder.sign = f1.decoder.sign ^ f2.decoder.sign; break;
    // unexpected
    default: BROADCAST(STAT_INSTR_EXCEPTION | ((u64)instr.raw << STAT_SHIFT_AMOUNT)); break;
    }

    core->fregs[rd] = val.i;
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