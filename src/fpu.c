#include "fpu.h"
#include <math.h>

typedef union converter {
    f32 f;
    u32 i;

    struct float_decoder {
        u32 body : 31;
        u32 sign : 1;
    } __attribute__((packed)) decoder;
} CVT;

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
    
    CVT f1 = { .i = core->fregs[rs1] };
    CVT f2 = { .i = core->fregs[rs2] };
    CVT val = { .f = f1.f + f2.f };

    core->fregs[rd] = val.i;
    core->pc += 4;
}

// fsub
void FSUB_EXEC(CORE* core, INSTR instr) {
    BYTE rd = instr.r.rd;
    BYTE rs1 = instr.r.rs1;
    BYTE rs2 = instr.r.rs2;
    
    CVT f1 = { .i = core->fregs[rs1] };
    CVT f2 = { .i = core->fregs[rs2] };
    CVT val = { .f = f1.f - f2.f };

    core->fregs[rd] = val.i;
    core->pc += 4;
}

// fmul
void FMUL_EXEC(CORE* core, INSTR instr) {
    BYTE rd = instr.r.rd;
    BYTE rs1 = instr.r.rs1;
    BYTE rs2 = instr.r.rs2;
    
    CVT f1 = { .i = core->fregs[rs1] };
    CVT f2 = { .i = core->fregs[rs2] };
    CVT val = { .f = f1.f * f2.f };

    core->fregs[rd] = val.i;
    core->pc += 4;
}

// fdiv
void FDIV_EXEC(CORE* core, INSTR instr) {
    BYTE rd = instr.r.rd;
    BYTE rs1 = instr.r.rs1;
    BYTE rs2 = instr.r.rs2;
    
    CVT f1 = { .i = core->fregs[rs1] };
    CVT f2 = { .i = core->fregs[rs2] };
    CVT val = { .f = f1.f / f2.f };

    core->fregs[rd] = val.i;
    core->pc += 4;
}

// fsqrt
void FSQRT_EXEC(CORE* core, INSTR instr) {
    BYTE rd = instr.r.rd;
    BYTE rs1 = instr.r.rs1;
    
    CVT f1 = { .i = core->fregs[rs1] };
    CVT val = { .f = sqrtf(f1.f) };

    core->fregs[rd] = val.i;
    core->pc += 4;
}

// fcmp
void FCMP_EXEC(CORE* core, INSTR instr) {
    BYTE rd = instr.r.rd;
    BYTE rs1 = instr.r.rs1;
    BYTE rs2 = instr.r.rs2;
    BYTE funct3 = instr.r.funct3;

    CVT f1 = { .i = core->fregs[rs1] };
    CVT f2 = { .i = core->fregs[rs2] };
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

    CVT f;
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

    CVT f = { .i = core->fregs[rs1] };
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

    CVT f1 = { .i = core->fregs[rs1] };
    CVT f2 = { .i = core->fregs[rs2] };
    CVT val = { .decoder = { .body = f2.decoder.body } };
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
