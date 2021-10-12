#pragma once
#include "types.h"
#include "core.h"

typedef union instr {
    uint32_t raw;

    struct _decoder {
        uint8_t opcode : 7;
        uint32_t operand : 25;
    }__attribute__((packed)) decoder;

    struct instr_r {
        uint8_t opcode : 7;
        uint8_t rd : 5;
        uint8_t funct3 : 3;
        uint8_t rs1 : 5;
        uint8_t rs2 : 5;
        uint8_t funct7 : 7;
    } __attribute__((packed)) r;

    struct instr_i {
        uint8_t opcode : 7;
        uint8_t rd : 5;
        uint8_t funct3 : 3;
        uint8_t rs1 : 5;
        uint32_t imm : 12;
    } __attribute__((packed)) i;

    struct instr_s {
        uint8_t opcode : 7;
        uint32_t imm4_0 : 5;
        uint8_t funct3 : 3;
        uint8_t rs1 : 5;
        uint8_t rs2 : 5;
        uint32_t imm11_5 : 7;
    } __attribute__((packed)) s;

    struct instr_b {
        uint8_t opcode : 7;
        uint32_t imm11 : 1;
        uint32_t imm4_1 : 4;
        uint8_t funct3 : 3;
        uint8_t rs1 : 5;
        uint8_t rs2 : 5;
        uint32_t imm10_5 : 6;
        uint32_t imm12 : 1;
    } __attribute__((packed)) b;

    struct instr_u {
        uint8_t opcode : 7;
        uint8_t rd : 5;
        uint32_t imm31_12 : 20;
    } __attribute__((packed)) u;

    struct instr_j {
        uint8_t opcode : 7;
        uint8_t rd : 5;
        uint32_t imm19_12 : 8;
        uint32_t imm11 : 1;
        uint32_t imm10_1 : 10;
        uint32_t imm20 : 1;
    } __attribute__((packed)) j;
} INSTR;

void execute(CORE *core, INSTR instr);
