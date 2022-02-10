#pragma once
#include "global.h"

#define sext(val, shift) ((val) | (((val) & (1 << (shift))) ? ~((1 << (shift)) - 1) : 0))

static char* instr_name[] = {
    "lui", "auipc",
    "jal", "jalr",
    "branch",
    "load", "store",
    "arith_i", "arith",
    "env/csr",
    "f-load", "f-store",
    "fmv2i", "fmv2f",
    "fadd", "fsub", "fmul", "fdiv", "fsqrt", "fcmp",
    "fcvt2f", "fcvt2i", "fsgnj", "f-branch",
    "v-load", "v-store"
};

enum instr_type {
    LUI = 0, AUIPC,
    JAL, JALR,
    BRANCH,
    LOAD, STORE,
    ARITH_I, ARITH,
    ENV_CSR,
    F_LOAD, F_STORE,
    FMV2I, FMV2F,
    FADD, FSUB, FMUL, FDIV, FSQRT, FCMP,
    FCVT2F, FCVT2I, FSGNJ, F_BRANCH,
    V_LOAD, V_STORE, UNDEFINED
};

typedef union instr {
    u32 raw;

    struct instr_decoder {
        u32 opcode : 7;
        u32 operand : 25;
    }__attribute__((packed)) decoder;

    struct instr_r {
        u32 opcode : 7;
        u32 rd : 5;
        u32 funct3 : 3;
        u32 rs1 : 5;
        u32 rs2 : 5;
        u32 funct7 : 7;
    } __attribute__((packed)) r;

    struct instr_i {
        u32 opcode : 7;
        u32 rd : 5;
        u32 funct3 : 3;
        u32 rs1 : 5;
        u32 imm : 12;
    } __attribute__((packed)) i;

    struct instr_s {
        u32 opcode : 7;
        u32 imm4_0 : 5;
        u32 funct3 : 3;
        u32 rs1 : 5;
        u32 rs2 : 5;
        u32 imm11_5 : 7;
    } __attribute__((packed)) s;

    struct instr_b {
        u32 opcode : 7;
        u32 imm11 : 1;
        u32 imm4_1 : 4;
        u32 funct3 : 3;
        u32 rs1 : 5;
        u32 rs2 : 5;
        u32 imm10_5 : 6;
        u32 imm12 : 1;
    } __attribute__((packed)) b;

    struct instr_u {
        u32 opcode : 7;
        u32 rd : 5;
        u32 imm31_12 : 20;
    } __attribute__((packed)) u;

    struct instr_j {
        u32 opcode : 7;
        u32 rd : 5;
        u32 imm19_12 : 8;
        u32 imm11 : 1;
        u32 imm10_1 : 10;
        u32 imm20 : 1;
    } __attribute__((packed)) j;

    struct instr_v1 {
        u32 opcode : 7;
        u32 mask : 4;
        u32 : 4;
        u32 r1 : 5;
        u32 imm : 12;
    } __attribute__((packed)) v1;

    struct instr_v2 {
        u32 opcode : 7;
        u32 : 1;
        u32 r5 : 6;
        u32 r4 : 6;
        u32 r3 : 6;
        u32 r2 : 6;
    } __attribute__((packed)) v2;
} INSTR;

u8 disasm(INSTR instr, char* buffer);
