#pragma once
#include "types.h"
#include "core.h"

typedef union instr {
    u_int32_t raw;

    struct _decoder {
        u_int8_t opcode : 7;
        u_int32_t operand : 25;
    }__attribute__((packed)) decoder;

    struct instr_r {
        u_int8_t opcode : 7;
        u_int8_t rd : 5;
        u_int8_t funct3 : 3;
        u_int8_t rs1 : 5;
        u_int8_t rs2 : 5;
        u_int8_t funct7 : 7;
    } __attribute__((packed)) r;

    struct instr_i {
        u_int8_t opcode : 7;
        u_int8_t rd : 5;
        u_int8_t funct3 : 3;
        u_int8_t rs1 : 5;
        u_int32_t imm : 12;
    } __attribute__((packed)) i;

    struct instr_s {
        u_int8_t opcode : 7;
        u_int32_t imm4_0 : 5;
        u_int8_t funct3 : 3;
        u_int8_t rs1 : 5;
        u_int8_t rs2 : 5;
        u_int32_t imm11_5 : 7;
    } __attribute__((packed)) s;

    struct instr_b {
        u_int8_t opcode : 7;
        u_int32_t imm11 : 1;
        u_int32_t imm4_1 : 4;
        u_int8_t funct3 : 3;
        u_int8_t rs1 : 5;
        u_int8_t rs2 : 5;
        u_int32_t imm10_5 : 6;
        u_int32_t imm12 : 1;
    } __attribute__((packed)) b;

    struct instr_u {
        u_int8_t opcode : 7;
        u_int8_t rd : 5;
        u_int32_t imm31_12 : 20;
    } __attribute__((packed)) u;

    struct instr_j {
        u_int8_t opcode : 7;
        u_int8_t rd : 5;
        u_int32_t imm19_12 : 8;
        u_int32_t imm11 : 1;
        u_int32_t imm10_1 : 10;
        u_int32_t imm20 : 1;
    } __attribute__((packed)) j;
} INSTR;

void execute(CORE *core, INSTR instr);
