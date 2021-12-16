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
void FMUL_EXEC(CORE* core, INSTR instr) {
    BYTE rd = instr.r.rd;
    BYTE rs1 = instr.r.rs1;
    BYTE rs2 = instr.r.rs2;

    FLOAT_HELPER x1 = { .i = core->fregs[rs1] };
    FLOAT_HELPER x2 = { .i = core->fregs[rs2] };
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
    core->fregs[rd] = val.i;
    core->pc += 4;
}

// fdiv
void FDIV_EXEC(CORE* core, INSTR instr) {
    BYTE rd = instr.r.rd;
    BYTE rs1 = instr.r.rs1;
    BYTE rs2 = instr.r.rs2;
    
    FLOAT_HELPER f1 = { .i = core->fregs[rs1] };
    FLOAT_HELPER f2 = { .i = core->fregs[rs2] };
    FLOAT_HELPER val = { .f = f1.f / f2.f };

    core->fregs[rd] = val.i;
    core->pc += 4;
}

// fsqrt
void FSQRT_EXEC(CORE* core, INSTR instr) {
    BYTE rd = instr.r.rd;
    BYTE rs1 = instr.r.rs1;
    
    FLOAT_HELPER f1 = { .i = core->fregs[rs1] };
    FLOAT_HELPER val = { .f = sqrtf(f1.f) };

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
    case 0b00000: i = (s32)f.f; break;
    // fcvt.wu.s
    case 0b00001: i = (u32)f.f; break;
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
