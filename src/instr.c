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
    sprintf(buffer, "jalr %s, %d(%s)", reg_name[rd], sext(imm, 11), reg_name[rs1]);
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
    // swi
    case 0b011: sprintf(buffer, "swi %s, %d(%s)", reg_name[rs2], sext(imm, 11), reg_name[rs1]); break;
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

    switch (funct3) {
    // env
    case 0b000: sprintf(buffer, imm ? "ebreak" : "ecall"); break;
    // unexpected
    default: sprintf(buffer, "unexpected env or csr"); break;
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

// f-mv to integer from float (loose check)
void FMV2I_DISASM(INSTR instr, char* buffer) {
    BYTE rd = instr.r.rd;
    BYTE rs1 = instr.r.rs1;
    sprintf(buffer, "fmv.x.w %s, %s", reg_name[rd], freg_name[rs1]);
}

// f-mv to float from integer (loose check)
void FMV2F_DISASM(INSTR instr, char* buffer) {
    BYTE rd = instr.r.rd;
    BYTE rs1 = instr.r.rs1;
    sprintf(buffer, "fmv.w.x %s, %s", freg_name[rd], reg_name[rs1]);
}

// fadd rd, rs1, rs2
void FADD_DISASM(INSTR instr, char* buffer) {
    BYTE rd = instr.r.rd;
    BYTE rs1 = instr.r.rs1;
    BYTE rs2 = instr.r.rs2;
    sprintf(buffer, "fadd %s, %s, %s", freg_name[rd], freg_name[rs1], freg_name[rs2]);
}

// fsub rd, rs1, rs2
void FSUB_DISASM(INSTR instr, char* buffer) {
    BYTE rd = instr.r.rd;
    BYTE rs1 = instr.r.rs1;
    BYTE rs2 = instr.r.rs2;
    sprintf(buffer, "fsub %s, %s, %s", freg_name[rd], freg_name[rs1], freg_name[rs2]);
}

// fmul rd, rs1, rs2
void FMUL_DISASM(INSTR instr, char* buffer) {
    BYTE rd = instr.r.rd;
    BYTE rs1 = instr.r.rs1;
    BYTE rs2 = instr.r.rs2;
    sprintf(buffer, "fmul %s, %s, %s", freg_name[rd], freg_name[rs1], freg_name[rs2]);
}

// fdiv rd, rs1, rs2
void FDIV_DISASM(INSTR instr, char* buffer) {
    BYTE rd = instr.r.rd;
    BYTE rs1 = instr.r.rs1;
    BYTE rs2 = instr.r.rs2;
    sprintf(buffer, "fdiv %s, %s, %s", freg_name[rd], freg_name[rs1], freg_name[rs2]);
}

// fsqrt rd, rs1
void FSQRT_DISASM(INSTR instr, char* buffer) {
    BYTE rd = instr.r.rd;
    BYTE rs1 = instr.r.rs1;
    sprintf(buffer, "fsqrt %s, %s", freg_name[rd], freg_name[rs1]);
}

// fcmp rd, rs1, rs2
void FCMP_DISASM(INSTR instr, char* buffer) {
    BYTE rd = instr.r.rd;
    BYTE rs1 = instr.r.rs1;
    BYTE rs2 = instr.r.rs2;
    BYTE funct3 = instr.r.funct3;
    switch (funct3) {
    case 0b010: sprintf(buffer, "feq %s, %s, %s", reg_name[rd], freg_name[rs1], freg_name[rs2]); break;
    case 0b001: sprintf(buffer, "flt %s, %s, %s", reg_name[rd], freg_name[rs1], freg_name[rs2]); break;
    case 0b000: sprintf(buffer, "fle %s, %s, %s", reg_name[rd], freg_name[rs1], freg_name[rs2]); break;
    default: sprintf(buffer, "unexpected load"); break;
    }
}

// fcvt2f rd, rs1
void FCVT2F_DISASM(INSTR instr, char* buffer) {
    BYTE rd = instr.r.rd;
    BYTE rs1 = instr.r.rs1;
    BYTE rs2 = instr.r.rs2;
    switch (rs2) {
    case 0b00000: sprintf(buffer, "fcvt.s.w %s, %s", freg_name[rd], reg_name[rs1]); break;
    case 0b00001: sprintf(buffer, "fcvt.s.wu %s, %s", freg_name[rd], reg_name[rs1]); break;
    default: sprintf(buffer, "unexpected load"); break;
    }
}

// fcvt2i rd, rs1
void FCVT2I_DISASM(INSTR instr, char* buffer) {
    BYTE rd = instr.r.rd;
    BYTE rs1 = instr.r.rs1;
    BYTE rs2 = instr.r.rs2;
    switch (rs2) {
    case 0b00000: sprintf(buffer, "fcvt.w.s %s, %s", reg_name[rd], freg_name[rs1]); break;
    case 0b00001: sprintf(buffer, "fcvt.wu.s %s, %s", reg_name[rd], freg_name[rs1]); break;
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
    case 0b000: sprintf(buffer, "fsgnj %s, %s, %s", freg_name[rd], freg_name[rs1], freg_name[rs2]); break;
    case 0b001: sprintf(buffer, "fsgnjn %s, %s, %s", freg_name[rd], freg_name[rs1], freg_name[rs2]); break;
    case 0b010: sprintf(buffer, "fsgnjx %s, %s, %s", freg_name[rd], freg_name[rs1], freg_name[rs2]); break;
    default: sprintf(buffer, "unexpected load"); break;
    }
}

u8 disasm(INSTR instr, char* buffer) {
    switch (instr.decoder.opcode) {
    /* RV32I + RV32M */
    // lui
    case 0b0110111: LUI_DISASM(instr, buffer); return LUI;
    // auipc
    case 0b0010111: AUIPC_DISASM(instr, buffer); return AUIPC;
    // jal
    case 0b1101111: JAL_DISASM(instr, buffer); return JAL;
    // jalr
    case 0b1100111: JALR_DISASM(instr, buffer); return JALR;
    // branch
    case 0b1100011: BRANCH_DISASM(instr, buffer); return BRANCH;
    // load
    case 0b0000011: LOAD_DISASM(instr, buffer); return LOAD;
    // store
    case 0b0100011: STORE_DISASM(instr, buffer); return STORE;
    // arith I
    case 0b0010011: ARITH_I_DISASM(instr, buffer); return ARITH_I;
    // arith
    case 0b0110011: ARITH_DISASM(instr, buffer); return ARITH;
    // fence
    case 0b0001111: sprintf(buffer, "unexpected instr (fence)"); return UNDEFINED;
    // env + csr
    case 0b1110011: ENV_DISASM(instr, buffer); return ENV_CSR;
    
    
    /* RV32F */
    // f-load
    case 0b0000111: FLW_DISASM(instr, buffer); return F_LOAD;
    // f-store
    case 0b0100111: FSW_DISASM(instr, buffer); return F_STORE;
    // f-arith (seperating for better analysis)
    case 0b1010011:
        switch (instr.r.funct7) {
        // f-mv to integer from float
        case 0b1110000: FMV2I_DISASM(instr, buffer); return FMV2I;
        // f-mv to float from integer
        case 0b1111000: FMV2F_DISASM(instr, buffer); return FMV2F;
        // fadd
        case 0b0000000: FADD_DISASM(instr, buffer); return FADD;
        // fsub
        case 0b0000100: FSUB_DISASM(instr, buffer); return FSUB;
        // fmul
        case 0b0001000: FMUL_DISASM(instr, buffer); return FMUL;
        // fdiv + fsqrt
        case 0b0001100:
            if (instr.r.rs2) {
                FDIV_DISASM(instr, buffer); return FDIV;
            } else {
                FSQRT_DISASM(instr, buffer); return FSQRT;
            }
        // f-cmp
        case 0b1010000: FCMP_DISASM(instr, buffer); return FCMP;
        // fcvt to integer from float
        case 0b1100000: FCVT2I_DISASM(instr, buffer); return FCVT2I;
        // fcvt to float from integer
        case 0b1101000: FCVT2F_DISASM(instr, buffer); return FCVT2F;
        // fsgnj
        case 0b0010000: FSGNJ_DISASM(instr, buffer); return FSGNJ;
        // unexpected
        default: sprintf(buffer, "unexpected instr"); return UNDEFINED;
        }
        break;
    // unexpected
    default: sprintf(buffer, "unexpected instr"); return UNDEFINED;
    }
}

u8 isLwStall(BYTE rd, WORD op) {
    INSTR instr = { .raw = op };
    char buffer[36];
    
    switch (disasm(instr, buffer)) {
    case JALR:
    case ARITH_I:
    case LOAD: case F_LOAD:
        return (rd == instr.i.rs1) ? 1 : 0;
    case BRANCH:
        return ((rd == instr.b.rs1) || (rd == instr.b.rs2)) ? 1 : 0;
    case STORE: case F_STORE:
        return ((rd == instr.s.rs1) || (rd == instr.s.rs2)) ? 1 : 0;
    case ARITH: case FADD: case FSUB: case FMUL: case FDIV:
    case FCMP:
    case FCVT2F: case FCVT2I:
    case FSGNJ:
        return ((rd == instr.r.rs1) || (rd == instr.r.rs2)) ? 1 : 0;
    case FMV2I: case FMV2F:
    case FSQRT:
        return (rd == instr.r.rs1) ? 1 : 0;
    default:
        return 0;
    }
}
