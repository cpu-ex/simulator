#pragma once
#include "types.h"

#define sext(val, shift) (val) | (((val) & (1 << (shift))) ? ~((1 << (shift)) - 1) : 0)

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
} INSTR;

void disasm(INSTR instr, char* buffer);
