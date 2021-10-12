#pragma once
#include "types.h"
#include "core.h"

typedef union instr {
    u32 raw;

    struct _decoder {
        u8 opcode : 7;
        u32 operand : 25;
    }__attribute__((packed)) decoder;

    struct instr_r {
        u8 opcode : 7;
        u8 rd : 5;
        u8 funct3 : 3;
        u8 rs1 : 5;
        u8 rs2 : 5;
        u8 funct7 : 7;
    } __attribute__((packed)) r;

    struct instr_i {
        u8 opcode : 7;
        u8 rd : 5;
        u8 funct3 : 3;
        u8 rs1 : 5;
        u32 imm : 12;
    } __attribute__((packed)) i;

    struct instr_s {
        u8 opcode : 7;
        u32 imm4_0 : 5;
        u8 funct3 : 3;
        u8 rs1 : 5;
        u8 rs2 : 5;
        u32 imm11_5 : 7;
    } __attribute__((packed)) s;

    struct instr_b {
        u8 opcode : 7;
        u32 imm11 : 1;
        u32 imm4_1 : 4;
        u8 funct3 : 3;
        u8 rs1 : 5;
        u8 rs2 : 5;
        u32 imm10_5 : 6;
        u32 imm12 : 1;
    } __attribute__((packed)) b;

    struct instr_u {
        u8 opcode : 7;
        u8 rd : 5;
        u32 imm31_12 : 20;
    } __attribute__((packed)) u;

    struct instr_j {
        u8 opcode : 7;
        u8 rd : 5;
        u32 imm19_12 : 8;
        u32 imm11 : 1;
        u32 imm10_1 : 10;
        u32 imm20 : 1;
    } __attribute__((packed)) j;
} INSTR;

void execute(CORE *core, INSTR instr);
