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

// arith rd, rs1, rs2 (RV32I + RV32M)
void ARITH_DISASM(INSTR instr, char* buffer) {
    BYTE rd = instr.r.rd;
    BYTE rs1 = instr.r.rs1;
    BYTE rs2 = instr.r.rs2;
    BYTE funct3 = instr.r.funct3;
    BYTE funct7 = instr.r.funct7;
    switch (funct3) {
    // add + sub + mul
    case 0b000:
        switch (funct7) {
            case 0b0000000: sprintf(buffer, "add %s, %s, %s", reg_name[rd], reg_name[rs1], reg_name[rs2]); break;
            case 0b0100000: sprintf(buffer, "sub %s, %s, %s", reg_name[rd], reg_name[rs1], reg_name[rs2]); break;
            case 0b0000001: sprintf(buffer, "mul %s, %s, %s", reg_name[rd], reg_name[rs1], reg_name[rs2]); break;
            default: sprintf(buffer, "unexpected arith"); break;
        }
        break;
    // sll
    case 0b001: sprintf(buffer, "sll %s, %s, %s", reg_name[rd], reg_name[rs1], reg_name[rs2]); break;
    // slt
    case 0b010: sprintf(buffer, "slt %s, %s, %s", reg_name[rd], reg_name[rs1], reg_name[rs2]); break;
    // sltu
    case 0b011: sprintf(buffer, "sltu %s, %s, %s", reg_name[rd], reg_name[rs1], reg_name[rs2]); break;
    // xor + div
    case 0b100:
        switch (funct7) {
            case 0b0000000: sprintf(buffer, "xor %s, %s, %s", reg_name[rd], reg_name[rs1], reg_name[rs2]); break;
            case 0b0000001: sprintf(buffer, "div %s, %s, %s", reg_name[rd], reg_name[rs1], reg_name[rs2]); break;
            default: sprintf(buffer, "unexpected arith"); break;
        }
        break;
    // srl + sra + divu
    case 0b101:
        switch (funct7) {
            case 0b0000000: sprintf(buffer, "srl %s, %s, %s", reg_name[rd], reg_name[rs1], reg_name[rs2]); break;
            case 0b0100000: sprintf(buffer, "sra %s, %s, %s", reg_name[rd], reg_name[rs1], reg_name[rs2]); break;
            case 0b0000001: sprintf(buffer, "divu %s, %s, %s", reg_name[rd], reg_name[rs1], reg_name[rs2]); break;
            default: sprintf(buffer, "unexpected arith"); break;
        }
        break;
    // or + rem
    case 0b110:
        switch (funct7) {
            case 0b0000000: sprintf(buffer, "or %s, %s, %s", reg_name[rd], reg_name[rs1], reg_name[rs2]); break;
            case 0b0000001: sprintf(buffer, "rem %s, %s, %s", reg_name[rd], reg_name[rs1], reg_name[rs2]); break;
            default: sprintf(buffer, "unexpected arith"); break;
        }
        break;
    // and + remu
    case 0b111:
        switch (funct7) {
            case 0b0000000: sprintf(buffer, "and %s, %s, %s", reg_name[rd], reg_name[rs1], reg_name[rs2]); break;
            case 0b0000001: sprintf(buffer, "remu %s, %s, %s", reg_name[rd], reg_name[rs1], reg_name[rs2]); break;
            default: sprintf(buffer, "unexpected arith"); break;
        }
        break;
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

// f-load rd, offset(rs1)
void FLW_DISASM(INSTR instr, char* buffer) {
    BYTE rd = instr.i.rd;
    WORD imm = instr.i.imm;
    BYTE rs1 = instr.i.rs1;
    BYTE funct3 = instr.i.funct3;
    switch (funct3) {
    // flw
    case 0b010: sprintf(buffer, "flw %s, %d(%s)", freg_name[rd], sext(imm, 11), reg_name[rs1]); break;
    // unexpected
    default: sprintf(buffer, "unexpected load"); break;
    }
}

// f-store rs2, offset(rs1)
void FSW_DISASM(INSTR instr, char* buffer) {
    WORD imm = instr.s.imm11_5 << 5 |
                instr.s.imm4_0;
    BYTE rs1 = instr.s.rs1;
    BYTE rs2 = instr.s.rs2;
    BYTE funct3 = instr.s.funct3;
    switch (funct3) {
    // fsw
    case 0b010: sprintf(buffer, "fsw %s, %d(%s)", freg_name[rs2], sext(imm, 11), reg_name[rs1]); break;
    // unexpected
    default: sprintf(buffer, "unexpected load"); break;
    }
}

// fadd rd, rs1, rs2
void FADD_DISASM(INSTR instr, char* buffer) {
    BYTE rd = instr.r.rd;
    BYTE rs1 = instr.r.rs1;
    BYTE rs2 = instr.r.rs2;
    sprintf("fadd %s, %s, %s", freg_name[rd], freg_name[rs1], freg_name[rs2]);
}

// fsub rd, rs1, rs2
void FSUB_DISASM(INSTR instr, char* buffer) {
    BYTE rd = instr.r.rd;
    BYTE rs1 = instr.r.rs1;
    BYTE rs2 = instr.r.rs2;
    sprintf("fsub %s, %s, %s", freg_name[rd], freg_name[rs1], freg_name[rs2]);
}

// fmul rd, rs1, rs2
void FMUL_DISASM(INSTR instr, char* buffer) {
    BYTE rd = instr.r.rd;
    BYTE rs1 = instr.r.rs1;
    BYTE rs2 = instr.r.rs2;
    sprintf("fmul %s, %s, %s", freg_name[rd], freg_name[rs1], freg_name[rs2]);
}

// fdiv rd, rs1, rs2
void FDIV_DISASM(INSTR instr, char* buffer) {
    BYTE rd = instr.r.rd;
    BYTE rs1 = instr.r.rs1;
    BYTE rs2 = instr.r.rs2;
    sprintf("fdiv %s, %s, %s", freg_name[rd], freg_name[rs1], freg_name[rs2]);
}

// fsqrt rd, rs1
void FSQRT_DISASM(INSTR instr, char* buffer) {
    BYTE rd = instr.r.rd;
    BYTE rs1 = instr.r.rs1;
    sprintf("fsqrt %s, %s", freg_name[rd], freg_name[rs1]);
}

// fcmp rd, rs1, rs2
void FCMP_DISASM(INSTR instr, char* buffer) {
    BYTE rd = instr.r.rd;
    BYTE rs1 = instr.r.rs1;
    BYTE rs2 = instr.r.rs2;
    BYTE funct3 = instr.r.funct3;
    switch (funct3) {
    case 0b010: sprintf("feq %s, %s, %s", freg_name[rd], freg_name[rs1], freg_name[rs2]); break;
    case 0b001: sprintf("flt %s, %s, %s", freg_name[rd], freg_name[rs1], freg_name[rs2]); break;
    case 0b000: sprintf("fle %s, %s, %s", freg_name[rd], freg_name[rs1], freg_name[rs2]); break;
    default: sprintf(buffer, "unexpected load"); break;
    }
}

// fcvt2s rd, rs1
void FCVT2S_DISASM(INSTR instr, char* buffer) {
    BYTE rd = instr.r.rd;
    BYTE rs1 = instr.r.rs1;
    BYTE rs2 = instr.r.rs2;
    switch (rs2) {
    case 0b00000: sprintf("fcvt.w.s %s, %s", freg_name[rd], freg_name[rs1]); break;
    case 0b00001: sprintf("fcvt.wu.s %s, %s", freg_name[rd], freg_name[rs1]); break;
    default: sprintf(buffer, "unexpected load"); break;
    }
}

// fcvt2w rd, rs1
void FCVT2W_DISASM(INSTR instr, char* buffer) {
    BYTE rd = instr.r.rd;
    BYTE rs1 = instr.r.rs1;
    BYTE rs2 = instr.r.rs2;
    switch (rs2) {
    case 0b00000: sprintf("fcvt.s.w %s, %s", freg_name[rd], freg_name[rs1]); break;
    case 0b00001: sprintf("fcvt.s.wu %s, %s", freg_name[rd], freg_name[rs1]); break;
    default: sprintf(buffer, "unexpected load"); break;
    }
}

// fsgnj rd, rs1, rs2
void FSGNJ_DISASM(INSTR instr, char* buffer) {
    BYTE rd = instr.r.rd;
    BYTE rs1 = instr.r.rs1;
    BYTE rs2 = instr.r.rs2;
    BYTE funct3 = instr.r.funct3;
    switch (funct3) {
    case 0b000: sprintf("fsgnj %s, %s, %s", freg_name[rd], freg_name[rs1], freg_name[rs2]); break;
    case 0b001: sprintf("fsgnjn %s, %s, %s", freg_name[rd], freg_name[rs1], freg_name[rs2]); break;
    case 0b010: sprintf("fsgnjx %s, %s, %s", freg_name[rd], freg_name[rs1], freg_name[rs2]); break;
    default: sprintf(buffer, "unexpected load"); break;
    }
}

void disasm(INSTR instr, char* buffer) {
    switch (instr.decoder.opcode) {
    /* RV32I + RV32M */
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
    
    
    /* RV32F */
    // f-load
    case 0b0000111: FLW_DISASM(instr, buffer); break;
    // f-store
    case 0b0100111: FSW_DISASM(instr, buffer); break;
    // f-arith (seperating for better analysis)
    case 0b1010011:
        switch (instr.r.funct7) {
        // fadd
        case 0b0000000: FADD_DISASM(instr, buffer); break;
        // fsub
        case 0b0000100: FSUB_DISASM(instr, buffer); break;
        // fmul
        case 0b0001000: FMUL_DISASM(instr, buffer); break;
        // fdiv + fsqrt
        case 0b0001100:
            if (instr.r.rs2) {
                FDIV_DISASM(instr, buffer);
            } else {
                FSQRT_DISASM(instr, buffer);
            }
            break;
        // f-cmp
        case 0b1010000: FCMP_DISASM(instr, buffer); break;
        // fcvt2s
        case 0b1100000: FCVT2S_DISASM(instr, buffer); break;
        // fcvt2w
        case 0b1101000: FCVT2W_DISASM(instr, buffer); break;
        // fsgnj
        case 0b0010000: FSGNJ_DISASM(instr, buffer); break;
        // unexpected
        default: sprintf(buffer, "unexpected instr"); break;
        }
        break;
    // unexpected
    default: sprintf(buffer, "unexpected instr"); break;
    }
}
