#include "exec.h"
#include "fpu.h"

// RV32I
#define ARITH_ADD(a1, a2) ((a1) + (a2))
#define ARITH_SUB(a1, a2) ((a1) - (a2))
#define ARITH_SLL(a1, a2) ((a1) << ((a2) & 0b11111)) // only take lower 5 bits as shift amount
#define ARITH_SLT(a1, a2) (((s32)(a1) < (s32)(a2)) ? 1 : 0)
#define ARITH_SLTU(a1, a2) (((a1) < (a2)) ? 1 : 0)
#define ARITH_XOR(a1, a2) ((a1) ^ (a2))
#define ARITH_SRL(a1, a2) ((a1) >> ((a2) & 0b11111)) // same with sll
#define ARITH_SRA(a1, a2) (u32)(((s32)(a1)) >> ((a2) & 0b11111)) // same with sll
#define ARITH_OR(a1, a2) ((a1) | (a2))
#define ARITH_AND(a1, a2) ((a1) & (a2))

void execute(CORE* const core, const INSTR instr) {
    register WORD rd, rs1, rs2, imm, funct3, funct7;
    register u32 tmp, a1, a2;

    switch (instr.decoder.opcode) {
    /* RV32I */
    // lui
    case 0b0110111:
        rd = instr.u.rd;
        imm = instr.u.imm31_12;
        core->regs[rd] = imm << 12;
        core->pc += 4;
        core->instr_analysis[LUI]++;
        break;
    // auipc
    case 0b0010111:
        rd = instr.u.rd;
        imm = instr.u.imm31_12;

        core->regs[rd] = core->pc + (imm << 12);
        core->pc += 4;
        core->instr_analysis[AUIPC]++;
        break;
    // jal
    case 0b1101111:
        rd = instr.j.rd;
        imm = instr.j.imm20 << 20 |
                instr.j.imm19_12 << 12 |
                instr.j.imm11 << 11 |
                instr.j.imm10_1 << 1;
        
        core->regs[rd] = core->pc + 4;
        core->pc += sext(imm, 20);
        // count stall
        core->stall_counter += 2;
        core->instr_analysis[JAL]++;
        break;
    // jalr
    case 0b1100111:
        rd = instr.i.rd;
        imm = instr.i.imm;
        rs1 = instr.i.rs1;
    
        register const WORD t = core->pc + 4;
        core->pc = (core->regs[rs1] + sext(imm, 11)) & ~1;
        core->regs[rd] = t;
        // count stall
        core->stall_counter += 2;
        core->instr_analysis[JALR]++;
        break;
    // branch
    case 0b1100011:
        imm = instr.b.imm12 << 12 |
                instr.b.imm11 << 11 |
                instr.b.imm10_5 << 5 |
                instr.b.imm4_1 << 1;
        rs1 = instr.b.rs1;
        rs2 = instr.b.rs2;
        funct3 = instr.b.funct3;
        
        a1 = core->regs[rs1];
        a2 = core->regs[rs2];
        switch (funct3) {
        // beq
        case 0b000: tmp = (a1 == a2) ? 1 : 0; break;
        // bne
        case 0b001: tmp = (a1 != a2) ? 1 : 0; break;
        // blt
        case 0b100: tmp = ((s32)a1 < (s32)a2) ? 1 : 0; break;
        // bge
        case 0b101: tmp = ((s32)a1 >= (s32)a2) ? 1 : 0; break;
        // bltu
        case 0b110: tmp = (a1 < a2) ? 1 : 0; break;
        // bgeu
        case 0b111: tmp = (a1 >= a2) ? 1 : 0; break;
        // unexpected
        default: BROADCAST(STAT_INSTR_EXCEPTION | ((u64)instr.raw << STAT_SHIFT_AMOUNT)); break;
        }
        // predict branch
        register const u32 predicted = core->branch_predictor->predict(core->branch_predictor, core->pc, tmp);
        // count stall
        core->stall_counter += (predicted == tmp) ? 0 : 2;
        // increment pc
        core->pc += tmp ? sext(imm, 12) : 4;
        core->instr_analysis[BRANCH]++;
        break;
    // load
    case 0b0000011:
        rd = instr.i.rd;
        imm = instr.i.imm;
        rs1 = instr.i.rs1;
        funct3 = instr.i.funct3;

        // funct3: 010 -> lw
        core->regs[rd] = core->load_data(core, core->regs[rs1] + sext(imm, 11));
        core->pc += 4;
        // count stall
        core->stall_counter += isLwStall(rd, core->load_instr(core, core->pc)) ? 1 : 0;
        core->instr_analysis[LOAD]++;
        break;
    // store
    case 0b0100011:
        imm = instr.s.imm11_5 << 5 | instr.s.imm4_0;
        rs1 = instr.s.rs1;
        rs2 = instr.s.rs2;
        funct3 = instr.s.funct3;

        // funct3: 011 -> swi, 010 -> sw
        if (funct3 == 0b011)
            core->store_instr(core, core->regs[rs1] + sext(imm, 11), core->regs[rs2]);
        else
            core->store_data(core, core->regs[rs1] + sext(imm, 11), core->regs[rs2]);
        core->pc += 4;
        core->instr_analysis[STORE]++;
        break;
    // arith I
    case 0b0010011:
        rd = instr.i.rd;
        imm = instr.i.imm;
        rs1 = instr.i.rs1;
        funct3 = instr.i.funct3;

        a1 = core->regs[rs1];
        a2 = sext(imm, 11);
        switch (funct3) {
        // addi
        case 0b000: core->regs[rd] = ARITH_ADD(a1, a2); break;
        // slli (legal when shamt[5] = 0, but not implemented)
        case 0b001: core->regs[rd] = ARITH_SLL(a1, a2); break;
        // slti
        case 0b010: core->regs[rd] = ARITH_SLT(a1, a2); break;
        // sltiu
        case 0b011: core->regs[rd] = ARITH_SLTU(a1, a2); break;
        // xori
        case 0b100: core->regs[rd] = ARITH_XOR(a1, a2); break;
        // srli + srai (same with slli)
        case 0b101: core->regs[rd] = (a2 >> 5) ? ARITH_SRA(a1, a2) : ARITH_SRL(a1, a2); break;
        // ori
        case 0b110: core->regs[rd] = ARITH_OR(a1, a2); break;
        // andi
        case 0b111: core->regs[rd] = ARITH_AND(a1, a2); break;
        // unexpected
        default: BROADCAST(STAT_INSTR_EXCEPTION | ((u64)instr.raw << STAT_SHIFT_AMOUNT)); break;
        }
        core->pc += 4;
        core->instr_analysis[ARITH_I]++;
        break;
    // arith
    case 0b0110011:
        rd = instr.r.rd;
        rs1 = instr.r.rs1;
        rs2 = instr.r.rs2;
        funct3 = instr.r.funct3;
        funct7 = instr.r.funct7;

        a1 = core->regs[rs1];
        a2 = core->regs[rs2];
        switch (funct3) {
        // add + sub + mul
        case 0b000: core->regs[rd] = funct7 ? ARITH_SUB(a1, a2) : ARITH_ADD(a1, a2); break;
        // sll
        case 0b001: core->regs[rd] = ARITH_SLL(a1, a2); break;
        // slt
        case 0b010: core->regs[rd] = ARITH_SLT(a1, a2); break;
        // sltu
        case 0b011: core->regs[rd] = ARITH_SLTU(a1, a2); break;
        // xor
        case 0b100: core->regs[rd] = ARITH_XOR(a1, a2); break;
        // srl + sra
        case 0b101: core->regs[rd] = funct7 ? ARITH_SRA(a1, a2) : ARITH_SRL(a1, a2); break;
        // or
        case 0b110: core->regs[rd] = ARITH_OR(a1, a2); break;
        // and
        case 0b111: core->regs[rd] = ARITH_AND(a1, a2); break;
        // unexpected
        default: BROADCAST(STAT_INSTR_EXCEPTION | ((u64)instr.raw << STAT_SHIFT_AMOUNT)); break;
        }
        core->pc += 4;
        core->instr_analysis[ARITH]++;
        break;
    // env + csr
    case 0b1110011:
        imm = instr.i.imm;
        funct3 = instr.i.funct3;
        if (funct3 == 0b000) {
            switch (imm) {
            // end the program
            case 0:
                BROADCAST(STAT_EXIT);
                break;
            // break now
            case 1:
                core->pc += 4;
                BROADCAST(STAT_HALT);
                break;
            // break after executing imm instructions
            default:
                core->store_instr(core, core->pc, (INSTR){ .i = {
                    .opcode = instr.i.opcode,
                    .rd     = instr.i.rd,
                    .funct3 = instr.i.funct3,
                    .rs1    = instr.i.rs1,
                    .imm    = max(1, imm - 1)
                } }.raw);
                core->pc += 4;
                break;
            }
        } else {
            // unexpected
            BROADCAST(STAT_INSTR_EXCEPTION | ((u64)instr.raw << STAT_SHIFT_AMOUNT));
        }
        core->instr_analysis[ENV_CSR]++;
        break;
    

    /* RV32F */
    // f-load
    case 0b0000111:
        rd = instr.i.rd;
        imm = instr.i.imm;
        rs1 = instr.i.rs1;

        core->fregs[rd] = core->load_data(core, core->regs[rs1] + sext(imm, 11));
        core->pc += 4;
        // count stall
        core->stall_counter += isLwStall(rd, core->load_instr(core, core->pc)) ? 1 : 0;
        core->instr_analysis[F_LOAD]++;
        break;
    // f-store
    case 0b0100111:
        imm = instr.s.imm11_5 << 5 | instr.s.imm4_0;
        rs1 = instr.s.rs1;
        rs2 = instr.s.rs2;

        core->store_data(core, core->regs[rs1] + sext(imm, 11), core->fregs[rs2]);
        core->pc += 4;
        core->instr_analysis[F_STORE]++;
        break;
    // f-arith (seprating for better analysis)
    case 0b1010011:
        switch (instr.r.funct7) {
        // f-mv to integer from float
        case 0b1110000:
            rd = instr.r.rd;
            rs1 = instr.r.rs1;

            core->regs[rd] = core->fregs[rs1];
            core->pc += 4;
            core->instr_analysis[FMV2I]++;
            break;
        // f-mv to float from integer
        case 0b1111000:
            rd = instr.r.rd;
            rs1 = instr.r.rs1;

            core->fregs[rd] = core->regs[rs1];
            core->pc += 4;
            core->instr_analysis[FMV2F]++;
            break;
        // fadd
        case 0b0000000: FADD_EXEC(core, instr); core->instr_analysis[FADD]++; break;
        // fsub
        case 0b0000100: FSUB_EXEC(core, instr); core->instr_analysis[FSUB]++; break;
        // fmul
        case 0b0001000: FMUL_EXEC(core, instr); core->instr_analysis[FMUL]++; break;
        // fdiv
        case 0b0001100: FDIV_EXEC(core, instr); core->instr_analysis[FDIV]++; break;
        // fsqrt
        case 0b0101100: FSQRT_EXEC(core, instr); core->instr_analysis[FSQRT]++; break;
        // fcmp
        case 0b1010000: FCMP_EXEC(core, instr); core->instr_analysis[FCMP]++; break;
        // fcvt to integer from float
        case 0b1100000: FCVT2I_EXEC(core, instr); core->instr_analysis[FCVT2I]++; break;
        // fcvt to float from integer
        case 0b1101000: FCVT2F_EXEC(core, instr); core->instr_analysis[FCVT2F]++; break;
        // fsgnj
        case 0b0010000: FSGNJ_EXEC(core, instr); core->instr_analysis[FSGNJ]++; break;
        // unexpected
        default: BROADCAST(STAT_INSTR_EXCEPTION | ((u64)instr.raw << STAT_SHIFT_AMOUNT)); break;
        }
        break;
    // unexpected
    default: BROADCAST(STAT_INSTR_EXCEPTION | ((u64)instr.raw << STAT_SHIFT_AMOUNT)); break;
    }
}
