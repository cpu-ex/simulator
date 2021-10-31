#include "instr.h"
#include "core.h"

// lui rd, imm
void LUI_DISASM(INSTR instr, char* buffer) {
    BYTE rd = instr.u.rd;
    WORD imm = instr.u.imm31_12;
    sprintf(buffer, "lui %s, %d", reg_name[rd], imm);
}

// auipc rd, imm
void AUIPC_DISASM(INSTR instr, char* buffer) {
    BYTE rd = instr.u.rd;
    WORD imm = instr.u.imm31_12;
    sprintf(buffer, "auipc %s, %d", reg_name[rd], imm);
}

// jal rd, imm
void JAL_DISASM(INSTR instr, char* buffer) {
    BYTE rd = instr.j.rd;
    WORD imm = instr.j.imm20 << 20 |
                instr.j.imm19_12 << 12 |
                instr.j.imm11 << 11 |
                instr.j.imm10_1 << 1;
    sprintf(buffer, "jal %s, %d", reg_name[rd], sext(imm, 20));
}

// jalr rd, rs1, imm
void JALR_DISASM(INSTR instr, char* buffer) {
    BYTE rd = instr.i.rd;
    WORD imm = instr.i.imm;
    BYTE rs1 = instr.i.rs1;
    sprintf(buffer, "jalr %s, %d(%s)", reg_name[rd], imm, reg_name[rs1]);
}

// branch rs1, rs2, imm
void BRANCH_DISASM(INSTR instr, char* buffer) {
    WORD imm = instr.b.imm12 << 12 |
                instr.b.imm11 << 11 |
                instr.b.imm10_5 << 5 |
                instr.b.imm4_1 << 1;
    BYTE rs1 = instr.b.rs1;
    BYTE rs2 = instr.b.rs2;
    BYTE funct3 = instr.b.funct3;
    switch (funct3) {
    // beq
    case 0b000: sprintf(buffer, "beq %s, %s, %d", reg_name[rs1], reg_name[rs2], sext(imm, 12)); break;
    // bne
    case 0b001: sprintf(buffer, "bne %s, %s, %d", reg_name[rs1], reg_name[rs2], sext(imm, 12)); break;
    // blt
    case 0b100: sprintf(buffer, "blt %s, %s, %d", reg_name[rs1], reg_name[rs2], sext(imm, 12)); break;
    // bge
    case 0b101: sprintf(buffer, "bge %s, %s, %d", reg_name[rs1], reg_name[rs2], sext(imm, 12)); break;
    // bltu
    case 0b110: sprintf(buffer, "bltu %s, %s, %d", reg_name[rs1], reg_name[rs2], sext(imm, 12)); break;
    // bgeu
    case 0b111: sprintf(buffer, "bgeu %s, %s, %d", reg_name[rs1], reg_name[rs2], sext(imm, 12)); break;
    // unexpected
    default: sprintf(buffer, "unexpected branch"); break;
    }
}

// load rd, imm(rs1)
void LOAD_DISASM(INSTR instr, char* buffer) {
    BYTE rd = instr.i.rd;
    WORD imm = instr.i.imm;
    BYTE rs1 = instr.i.rs1;
    BYTE funct3 = instr.i.funct3;
    switch (funct3) {
    // lb
    case 0b000: sprintf(buffer, "lb %s, %d(%s)", reg_name[rd], sext(imm, 11), reg_name[rs1]); break;
    // lh
    case 0b001: sprintf(buffer, "lh %s, %d(%s)", reg_name[rd], sext(imm, 11), reg_name[rs1]); break;
    // lw
    case 0b010: sprintf(buffer, "lw %s, %d(%s)", reg_name[rd], sext(imm, 11), reg_name[rs1]); break;
    // lbu
    case 0b100: sprintf(buffer, "lbu %s, %d(%s)", reg_name[rd], sext(imm, 11), reg_name[rs1]); break;
    // lhu
    case 0b101: sprintf(buffer, "lhu %s, %d(%s)", reg_name[rd], sext(imm, 11), reg_name[rs1]); break;
    // unexpected
    default: sprintf(buffer, "unexpected load"); break;
    }
}

// store rs2, offset(rs1)
void STORE_DISASM(INSTR instr, char* buffer) {
    WORD imm = instr.s.imm11_5 << 5 |
                instr.s.imm4_0;
    BYTE rs1 = instr.s.rs1;
    BYTE rs2 = instr.s.rs2;
    BYTE funct3 = instr.s.funct3;
    switch (funct3) {
    // sb
    case 0b000: sprintf(buffer, "sb %s, %d(%s)", reg_name[rs2], sext(imm, 11), reg_name[rs1]); break;
    // sh
    case 0b001: sprintf(buffer, "sh %s, %d(%s)", reg_name[rs2], sext(imm, 11), reg_name[rs1]); break;
    // sw
    case 0b010: sprintf(buffer, "sw %s, %d(%s)", reg_name[rs2], sext(imm, 11), reg_name[rs1]); break;
    // unexpected
    default: sprintf(buffer, "unexpected store"); break;
    }
}

// arith_i rd, rs1, imm
void ARITH_I_DISASM(INSTR instr, char* buffer) {
    BYTE rd = instr.i.rd;
    WORD imm = instr.i.imm;
    BYTE rs1 = instr.i.rs1;
    BYTE funct3 = instr.i.funct3;
    switch (funct3) {
    // addi
    case 0b000: sprintf(buffer, "addi %s, %s, %d", reg_name[rd], reg_name[rs1], sext(imm, 11)); break;
    // slli
    case 0b001: sprintf(buffer, "slli %s, %s, %d", reg_name[rd], reg_name[rs1], imm & 0x1F); break;
    // slti
    case 0b010: sprintf(buffer, "slti %s, %s, %d", reg_name[rd], reg_name[rs1], sext(imm, 11)); break;
    // sltiu
    case 0b011: sprintf(buffer, "sltiu %s, %s, %d", reg_name[rd], reg_name[rs1], sext(imm, 11)); break;
    // xori
    case 0b100: sprintf(buffer, "xori %s, %s, %d", reg_name[rd], reg_name[rs1], sext(imm, 11)); break;
    // srli + srai
    case 0b101:
        if (imm & 0xFE0) {
            sprintf(buffer, "srai %s, %s, %d", reg_name[rd], reg_name[rs1], imm & 0x1F);
        } else {
            sprintf(buffer, "srli %s, %s, %d", reg_name[rd], reg_name[rs1], imm & 0x1F);
        }
        break;
    // ori
    case 0b110: sprintf(buffer, "ori %s, %s, %d", reg_name[rd], reg_name[rs1], sext(imm, 11)); break;
    // andi
    case 0b111: sprintf(buffer, "andi %s, %s, %d", reg_name[rd], reg_name[rs1], sext(imm, 11)); break;
    // unexpected
    default: sprintf(buffer, "unexpected arith with imm"); break;
    }
}

// arith rd, rs1, rs2
void ARITH_DISASM(INSTR instr, char* buffer) {
    BYTE rd = instr.r.rd;
    BYTE rs1 = instr.r.rs1;
    BYTE rs2 = instr.r.rs2;
    BYTE funct3 = instr.r.funct3;
    BYTE funct7 = instr.r.funct7;
    switch (funct3) {
    // add + sub
    case 0b000:
        if (funct7) {
            sprintf(buffer, "sub %s, %s, %s", reg_name[rd], reg_name[rs1], reg_name[rs2]);
        } else {
            sprintf(buffer, "add %s, %s, %s", reg_name[rd], reg_name[rs1], reg_name[rs2]);
        }
        break;
    // sll
    case 0b001: sprintf(buffer, "sll %s, %s, %s", reg_name[rd], reg_name[rs1], reg_name[rs2]); break;
    // slt
    case 0b010: sprintf(buffer, "slt %s, %s, %s", reg_name[rd], reg_name[rs1], reg_name[rs2]); break;
    // sltu
    case 0b011: sprintf(buffer, "sltu %s, %s, %s", reg_name[rd], reg_name[rs1], reg_name[rs2]); break;
    // xor
    case 0b100: sprintf(buffer, "xor %s, %s, %s", reg_name[rd], reg_name[rs1], reg_name[rs2]); break;
    // srl + sra
    case 0b101:
        if (funct7) {
            sprintf(buffer, "sra %s, %s, %s", reg_name[rd], reg_name[rs1], reg_name[rs2]);
        } else {
            sprintf(buffer, "srl %s, %s, %s", reg_name[rd], reg_name[rs1], reg_name[rs2]);
        }
        break;
    // or
    case 0b110: sprintf(buffer, "or %s, %s, %s", reg_name[rd], reg_name[rs1], reg_name[rs2]); break;
    // and
    case 0b111: sprintf(buffer, "and %s, %s, %s", reg_name[rd], reg_name[rs1], reg_name[rs2]); break;
    // unexpected
    default: sprintf(buffer, "unexpected arith"); break;
    }
}

// env: ebreak
void ENV_DISASM(INSTR instr, char* buffer) {
    WORD imm = instr.i.imm;
    BYTE funct3 = instr.i.funct3;

    if ((imm == 1) && (funct3 == 0)) {
        // ebreak
        sprintf(buffer, "ebreak");
    } else {
        // not implemented
        sprintf(buffer, "unexpected env or csr");
    }
}

void disasm(INSTR instr, char* buffer) {
    switch (instr.decoder.opcode) {
    /* risc-v I */
    // lui
    case 0b0110111: LUI_DISASM(instr, buffer); break;
    // auipc
    case 0b0010111: AUIPC_DISASM(instr, buffer); break;
    // jal
    case 0b1101111: JAL_DISASM(instr, buffer); break;
    // jalr
    case 0b1100111: JALR_DISASM(instr, buffer); break;
    // branch
    case 0b1100011: BRANCH_DISASM(instr, buffer); break;
    // load
    case 0b0000011: LOAD_DISASM(instr, buffer); break;
    // store
    case 0b0100011: STORE_DISASM(instr, buffer); break;
    // arith I
    case 0b0010011: ARITH_I_DISASM(instr, buffer); break;
    // arith
    case 0b0110011: ARITH_DISASM(instr, buffer);break;
    // fence
    case 0b0001111: sprintf(buffer, "unexpected instr (fence)"); break;
    // env + csr
    case 0b1110011: ENV_DISASM(instr, buffer); break;
    // unexpected
    default: sprintf(buffer, "unexpected instr"); break;
    }
}
