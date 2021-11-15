#include "exec.h"
#include "fpu.h"

// lui: load upper immediate
void LUI_EXEC(CORE* core, INSTR instr) {
    BYTE rd = instr.u.rd;
    WORD imm = instr.u.imm31_12;

    core->regs[rd] = imm << 12;
    core->pc += 4;
}

// auipc: add upper immediate to pc
void AUIPC_EXEC(CORE* core, INSTR instr) {
    BYTE rd = instr.u.rd;
    WORD imm = instr.u.imm31_12;

    core->regs[rd] = core->pc + (imm << 12);
    core->pc += 4;
}

// jal: jump and link
void JAL_EXEC(CORE* core, INSTR instr) {
    BYTE rd = instr.j.rd;
    WORD imm = instr.j.imm20 << 20 |
                instr.j.imm19_12 << 12 |
                instr.j.imm11 << 11 |
                instr.j.imm10_1 << 1;

    core->regs[rd] = core->pc + 4;
    core->pc += sext(imm, 20);
}

// jalr: jump and link register
void JALR_EXEC(CORE* core, INSTR instr) {
    BYTE rd = instr.i.rd;
    WORD imm = instr.i.imm;
    BYTE rs1 = instr.i.rs1;
    
    REG t = core->pc + 4;
    core->pc = (core->regs[rs1] + sext(imm, 11)) & ~1;
    core->regs[rd] = t;
}

// branch: beq, bne, blt, bge, bltu, bgeu
void BRANCH_EXEC(CORE* core, INSTR instr) {
    WORD imm = instr.b.imm12 << 12 |
                instr.b.imm11 << 11 |
                instr.b.imm10_5 << 5 |
                instr.b.imm4_1 << 1;
    BYTE rs1 = instr.b.rs1;
    BYTE rs2 = instr.b.rs2;
    BYTE funct3 = instr.b.funct3;

    u8 cmp = 0;
    WORD a1 = core->regs[rs1], a2 = core->regs[rs2];
    switch (funct3) {
    // beq
    case 0b000: cmp = (a1 == a2) ? 1 : 0; break;
    // bne
    case 0b001: cmp = (a1 != a2) ? 1 : 0; break;
    // blt
    case 0b100: cmp = ((signed)a1 < (signed)a2) ? 1 : 0; break;
    // bge
    case 0b101: cmp = ((signed)a1 >= (signed)a2) ? 1 : 0; break;
    // bltu
    case 0b110: cmp = (a1 < a2) ? 1 : 0; break;
    // bgeu
    case 0b111: cmp = (a1 >= a2) ? 1 : 0; break;
    // unexpected
    default: BROADCAST(STAT_INSTR_EXCEPTION | ((u64)instr.raw << 32)); break;
    }
    core->pc += cmp ? sext(imm, 12) : 4;
}

// load: lb, lh, lw, lbu, lhu
void LOAD_EXEC(CORE* core, INSTR instr) {
    BYTE rd = instr.i.rd;
    WORD imm = instr.i.imm;
    BYTE rs1 = instr.i.rs1;
    BYTE funct3 = instr.i.funct3;

    // funct3: 000 -> lb, 001 -> lh, 010 -> lw
    //         100 -> lbu, 101 -> lhu
    int bytes = funct3 & 0b011;
    int sign = !(funct3 >> 2);
    WORD val = core->load_data(core->regs[rs1] + sext(imm, 11), bytes, sign);
    core->regs[rd] = val;
    core->pc += 4;
}

// store: sb, sh, sw
void STORE_EXEC(CORE* core, INSTR instr) {
    WORD imm = instr.s.imm11_5 << 5 |
                instr.s.imm4_0;
    BYTE rs1 = instr.s.rs1;
    BYTE rs2 = instr.s.rs2;
    BYTE funct3 = instr.s.funct3;

    if (funct3 == 0b011)
        core->store_instr(core->regs[rs1] + sext(imm, 11), core->regs[rs2]);
    else // funct3: 000 -> sb, 001 -> sh, 010 -> sw
        core->store_data(core->regs[rs1] + sext(imm, 11), core->regs[rs2], funct3);
    core->pc += 4;
}

// RV32I
#define ARITH_ADD(a1, a2) (a1) + (a2)
#define ARITH_SUB(a1, a2) (a1) - (a2)
#define ARITH_SLL(a1, a2) ((a1) << ((a2) & 0b11111)) // only take lower 5 bits as shift amount
#define ARITH_SLT(a1, a2) (((signed)(a1) < (signed)(a2)) ? 1 : 0)
#define ARITH_SLTU(a1, a2) (((a1) < (a2)) ? 1 : 0)
#define ARITH_XOR(a1, a2) ((a1) ^ (a2))
#define ARITH_SRL(a1, a2) ((a1) >> ((a2) & 0b11111)) // same with sll
#define ARITH_SRA(a1, a2) (unsigned)(((signed)(a1)) >> ((a2)&0b11111)) // same with sll
#define ARITH_OR(a1, a2) ((a1) | (a2))
#define ARITH_AND(a1, a2) ((a1) & (a2))
// RV32M
#define ARITH_MUL(a1, a2) (a1) * (a2)
#define ARITH_DIV(a1, a2) ((signed)(a1)) / ((signed)(a2))
#define ARITH_DIVU(a1, a2) (a1) / (a2)
#define ARITH_REM(a1, a2) ((signed)(a1)) % ((signed)(a2))
#define ARITH_REMU(a1, a2) (a1) % (a2)

// arith with immediate variants
// addi, slli, slti, sltiu, xori, srli, srai, ori, andi
void ARITH_I_EXEC(CORE* core, INSTR instr) {
    BYTE rd = instr.i.rd;
    WORD imm = instr.i.imm;
    BYTE rs1 = instr.i.rs1;
    BYTE funct3 = instr.i.funct3;

    WORD val = 0;
    WORD a1 = core->regs[rs1], a2 = sext(imm, 11);
    switch (funct3) {
    // addi
    case 0b000: val = ARITH_ADD(a1, a2); break;
    // slli (legal when shamt[5] = 0, but not implemented)
    case 0b001: val = ARITH_SLL(a1, a2); break;
    // slti
    case 0b010: val = ARITH_SLT(a1, a2); break;
    // sltiu
    case 0b011: val = ARITH_SLTU(a1, a2); break;
    // xori
    case 0b100: val = ARITH_XOR(a1, a2); break;
    // srli + srai (same with slli)
    case 0b101:
        switch (a2 >> 5) {
            case 0b0000000: val = ARITH_SRL(a1, a2); break;
            case 0b0100000: val = ARITH_SRA(a1, a2); break;
            default: BROADCAST(STAT_INSTR_EXCEPTION | ((u64)instr.raw << 32)); break;
        }
        break;
    // ori
    case 0b110: val = ARITH_OR(a1, a2); break;
    // andi
    case 0b111: val = ARITH_AND(a1, a2); break;
    // unexpected
    default: BROADCAST(STAT_INSTR_EXCEPTION | ((u64)instr.raw << 32)); break;
    }
    core->regs[rd] = val;
    core->pc += 4;
}

// arith (RV32I + RV32M)
// add, sub, sll, slt, sltu, xor, srl, sra, or, and
void ARITH_EXEC(CORE* core, INSTR instr) {
    BYTE rd = instr.r.rd;
    BYTE rs1 = instr.r.rs1;
    BYTE rs2 = instr.r.rs2;
    BYTE funct3 = instr.r.funct3;
    BYTE funct7 = instr.r.funct7;

    WORD val = 0;
    WORD a1 = core->regs[rs1], a2 = core->regs[rs2];
    switch (funct3) {
    // add + sub + mul
    case 0b000:
        switch (funct7) {
            case 0b0000000: val = ARITH_ADD(a1, a2); break;
            case 0b0100000: val = ARITH_SUB(a1, a2); break;
            case 0b0000001: val = ARITH_MUL(a1, a2); break;
            default: BROADCAST(STAT_INSTR_EXCEPTION | ((u64)instr.raw << 32)); break;
        }
        break;
    // sll
    case 0b001: val = ARITH_SLL(a1, a2); break;
    // slt
    case 0b010: val = ARITH_SLT(a1, a2); break;
    // sltu
    case 0b011: val = ARITH_SLTU(a1, a2); break;
    // xor + div
    case 0b100:
        switch (funct7) {
            case 0b0000000: val = ARITH_XOR(a1, a2); break;
            case 0b0000001: val = ARITH_DIV(a1, a2); break;
            default: BROADCAST(STAT_INSTR_EXCEPTION | ((u64)instr.raw << 32)); break;
        }
        break;
    // srl + sra + divu
    case 0b101:
        switch (funct7) {
            case 0b0000000: val = ARITH_SRL(a1, a2); break;
            case 0b0100000: val = ARITH_SRA(a1, a2); break;
            case 0b0000001: val = ARITH_DIVU(a1, a2); break;
            default: BROADCAST(STAT_INSTR_EXCEPTION | ((u64)instr.raw << 32)); break;
        }
        break;
    // or + rem
    case 0b110:
        switch (funct7) {
            case 0b0000000: val = ARITH_OR(a1, a2); break;
            case 0b0000001: val = ARITH_REM(a1, a2); break;
            default: BROADCAST(STAT_INSTR_EXCEPTION | ((u64)instr.raw << 32)); break;
        }
        break;
    // and + remu
    case 0b111:
        switch (funct7) {
            case 0b0000000: val = ARITH_AND(a1, a2); break;
            case 0b0000001: val = ARITH_REMU(a1, a2); break;
            default: BROADCAST(STAT_INSTR_EXCEPTION | ((u64)instr.raw << 32)); break;
        }
        break;
    // unexpected
    default: BROADCAST(STAT_INSTR_EXCEPTION | ((u64)instr.raw << 32)); break;
    }
    core->regs[rd] = val;
    core->pc += 4;
}

// env: ebreak
void ENV_EXEC(CORE* core, INSTR instr) {
    WORD imm = instr.i.imm;
    BYTE funct3 = instr.i.funct3;

    switch (funct3) {
    // env
    if (imm) {
        // ebreak
        BROADCAST(STAT_EXIT);
    } else {
        // ecall
        printf("ecall here");
        core->pc = DEFAULT_PC;
        BROADCAST(STAT_HALT);
    }
    // unexpected
    default: BROADCAST(STAT_INSTR_EXCEPTION | ((u64)instr.raw << 32)); break;
    }
}

void execute(CORE* core, INSTR instr) {
    switch (instr.decoder.opcode) {
    /* RV32I + RV32M */
    // lui
    case 0b0110111:
        LUI_EXEC(core, instr);
        core->instr_analysis[LUI]++;
        break;
    // auipc
    case 0b0010111:
        AUIPC_EXEC(core, instr);
        core->instr_analysis[AUIPC]++;
        break;
    // jal
    case 0b1101111:
        JAL_EXEC(core, instr);
        core->instr_analysis[JAL]++;
        break;
    // jalr
    case 0b1100111:
        JALR_EXEC(core, instr);
        core->instr_analysis[JALR]++;
        break;
    // branch
    case 0b1100011:
        BRANCH_EXEC(core, instr);
        core->instr_analysis[BRANCH]++;
        break;
    // load
    case 0b0000011:
        LOAD_EXEC(core, instr);
        core->instr_analysis[LOAD]++;
        break;
    // store
    case 0b0100011:
        STORE_EXEC(core, instr);
        core->instr_analysis[STORE]++;
        break;
    // arith I
    case 0b0010011:
        ARITH_I_EXEC(core, instr);
        core->instr_analysis[ARITH_I]++;
        break;
    // arith
    case 0b0110011:
        ARITH_EXEC(core, instr);
        core->instr_analysis[ARITH]++;
        break;
    // fence
    case 0b0001111:
        BROADCAST(STAT_INSTR_EXCEPTION | ((u64)instr.raw << 32));
        break;
    // env + csr
    case 0b1110011:
        ENV_EXEC(core, instr);
        core->instr_analysis[ENV_CSR]++;
        break;
    

    /* RV32F */
    // f-load
    case 0b0000111:
        FLW_EXEC(core, instr);
        core->instr_analysis[F_LOAD]++;
        break;
    // f-store
    case 0b0100111:
        FSW_EXEC(core, instr);
        core->instr_analysis[F_STORE]++;
        break;
    // f-arith (seprating for better analysis)
    case 0b1010011:
        switch (instr.r.funct7) {
        // f-mv to integer from float
        case 0b1110000:
            FMV2I_EXEC(core, instr);
            core->instr_analysis[FMV2I]++;
            break;
        // f-mv to float from integer
        case 0b1111000:
            FMV2F_EXEC(core, instr);
            core->instr_analysis[FMV2F]++;
            break;
        // fadd
        case 0b0000000:
            FADD_EXEC(core, instr);
            core->instr_analysis[FADD]++;
            break;
        // fsub
        case 0b0000100:
            FSUB_EXEC(core, instr);
            core->instr_analysis[FSUB]++;
            break;
        // fmul
        case 0b0001000:
            FMUL_EXEC(core, instr);
            core->instr_analysis[FMUL]++;
            break;
        // fdiv + fsqrt
        case 0b0001100:
            if (instr.r.rs2) {
                FDIV_EXEC(core, instr);
                core->instr_analysis[FDIV]++;
            } else {
                FSQRT_EXEC(core, instr);
                core->instr_analysis[FSQRT]++;
            }
            break;
        // fcmp
        case 0b1010000:
            FCMP_EXEC(core, instr);
            core->instr_analysis[FCMP]++;
            break;
        // fcvt to integer from float
        case 0b1100000:
            FCVT2I_EXEC(core, instr);
            core->instr_analysis[FCVT2I]++;
            break;
        // fcvt to float from integer
        case 0b1101000:
            FCVT2F_EXEC(core, instr);
            core->instr_analysis[FCVT2F]++;
            break;
        // fsgnj
        case 0b0010000:
            FSGNJ_EXEC(core, instr);
            core->instr_analysis[FSGNJ]++;
            break;
        // unexpected
        default:
            BROADCAST(STAT_INSTR_EXCEPTION | ((u64)instr.raw << 32));
            break;
        }
        break;
    // unexpected
    default:
        BROADCAST(STAT_INSTR_EXCEPTION | ((u64)instr.raw << 32));
        break;
    }
}
