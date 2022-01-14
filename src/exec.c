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

#define get_lw_stall(rd, op) ((rd == 0) ? 0 : ((rd == ((op >> 15) & 0x1F)) | (rd == ((op >> 20) & 0x1F))) ? 1 : 0)

void execute(CORE* const core, const INSTR instr) {
    register const WORD rd = instr.r.rd;
    register const WORD rs1 = instr.r.rs1;
    register const WORD rs2 = instr.r.rs2;
    register const WORD funct3 = instr.r.funct3;
    register const WORD funct7 = instr.r.funct7;

    register u32 imm, tmp, a1, a2;
    register FLOAT_HELPER f1, f2;

    switch (instr.decoder.opcode) {
    // arith I
    case 0b0010011:
        imm = instr.i.imm;
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
        ++core->instr_analysis[ARITH_I];
        break;
    // load
    case 0b0000011:
        imm = instr.i.imm;
        // funct3: 010 -> lw
        core->regs[rd] = core->load_data(core, core->regs[rs1] + sext(imm, 11));
        core->pc += 4;
        // rudely fetch next instr and count stall
        tmp = core->mmu->instr_mem[core->pc >> 2]; // core->load_instr(core, core->pc)
        core->stall_counter += get_lw_stall(rd, tmp);
        ++core->instr_analysis[LOAD];
        break;
    // store
    case 0b0100011:
        imm = instr.s.imm11_5 << 5 | instr.s.imm4_0;
        // funct3: 011 -> swi, 010 -> sw
        if (funct3 == 0b011)
            core->store_instr(core, core->regs[rs1] + sext(imm, 11), core->regs[rs2]);
        else
            core->store_data(core, core->regs[rs1] + sext(imm, 11), core->regs[rs2]);
        core->pc += 4;
        ++core->instr_analysis[STORE];
        break;
    // arith
    case 0b0110011:
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
        ++core->instr_analysis[ARITH];
        break;
    // f-load
    case 0b0000111:
        imm = instr.i.imm;
        core->fregs[rd] = core->load_data(core, core->regs[rs1] + sext(imm, 11));
        core->pc += 4;
        // rudely fetch next instr and count stall
        tmp = core->mmu->instr_mem[core->pc >> 2]; // core->load_instr(core, core->pc)
        core->stall_counter += get_lw_stall(rd, tmp);
        ++core->instr_analysis[F_LOAD];
        break;
    // f-arith (seprating for better analysis)
    case 0b1010011:
        switch (funct7) {
        // fmul
        case 0b0001000:
            core->fregs[rd] = fmul(
                (FLOAT_HELPER){ .i = core->fregs[rs1] },
                (FLOAT_HELPER){ .i = core->fregs[rs2] }
            ).i;
            core->pc += 4;
            ++core->instr_analysis[FMUL];
            break;
        // fcmp
        case 0b1010000:
            f1.i = core->fregs[rs1];
            f2.i = core->fregs[rs2];
            switch (funct3) {
            // fle
            case 0b000: core->regs[rd] = (f1.f <= f2.f) ? 1 : 0; break;
            // flt
            case 0b001: core->regs[rd] = (f1.f < f2.f) ? 1 : 0; break;
            // feq
            case 0b010: core->regs[rd] = (f1.f == f2.f) ? 1 : 0; break;
            // unexpected
            default: BROADCAST(STAT_INSTR_EXCEPTION | ((u64)instr.raw << STAT_SHIFT_AMOUNT)); break;
            }
            core->pc += 4;
            ++core->instr_analysis[FCMP];
            break;
        // fadd
        case 0b0000000:
            core->fregs[rd] = (FLOAT_HELPER){
                .f = (FLOAT_HELPER){
                    .i = core->fregs[rs1]
                }.f + (FLOAT_HELPER){
                    .i = core->fregs[rs2]
                }.f
            }.i;
            core->pc += 4;
            ++core->instr_analysis[FADD];
            break;
        // fsub
        case 0b0000100:
            core->fregs[rd] = (FLOAT_HELPER){
                .f = (FLOAT_HELPER){
                    .i = core->fregs[rs1]
                }.f - (FLOAT_HELPER){
                    .i = core->fregs[rs2]
                }.f
            }.i;
            core->pc += 4;
            ++core->instr_analysis[FSUB];
            break;
        // fsgnj
        case 0b0010000:
            f1.i = core->fregs[rs1];
            f2.i = core->fregs[rs2];
            switch (funct3) {
            // fsgnj
            case 0b000: f1.decoder.sign = f2.decoder.sign; break;
            // fsgnjn
            case 0b001: f1.decoder.sign = ~f2.decoder.sign; break;
            // fsgnjx
            case 0b010: f1.decoder.sign ^= f2.decoder.sign; break;
            // unexpected
            default: BROADCAST(STAT_INSTR_EXCEPTION | ((u64)instr.raw << STAT_SHIFT_AMOUNT)); break;
            }
            core->fregs[rd] = f1.i;
            core->pc += 4;
            ++core->instr_analysis[FSGNJ];
            break;
        // f-mv to float from integer
        case 0b1111000:
            core->fregs[rd] = core->regs[rs1];
            core->pc += 4;
            ++core->instr_analysis[FMV2F];
            break;
        // fsqrt
        case 0b0101100:
            core->fregs[rd] = fsqrt((FLOAT_HELPER){ .i = core->fregs[rs1] }).i;
            core->pc += 4;
            ++core->instr_analysis[FSQRT];
            break;
        // fdiv
        case 0b0001100:
            core->fregs[rd] = fdiv(
                (FLOAT_HELPER){ .i = core->fregs[rs1] },
                (FLOAT_HELPER){ .i = core->fregs[rs2] }
            ).i;
            core->pc += 4;
            ++core->instr_analysis[FDIV];
            break;
        // fcvt to integer from float
        case 0b1100000:
            f1.i = core->fregs[rs1];
            switch (rs2) {
            // fcvt.w.s
            case 0b00000: core->regs[rd] = (f1.f < 0.0) ? ((s32)(f1.f - (f32)((s32)(f1.f - 1.0)) + 0.5) + (s32)(f1.f - 1.0)) : ((s32)(f1.f + 0.5)); break;
            // fcvt.wu.s
            case 0b00001: core->regs[rd] = (u32)(f1.f + 0.5); break;
            // unexpected
            default: BROADCAST(STAT_INSTR_EXCEPTION | ((u64)instr.raw << STAT_SHIFT_AMOUNT)); break;
            }
            core->pc += 4;
            ++core->instr_analysis[FCVT2I];
            break;
        // fcvt to float from integer
        case 0b1101000:
            switch (rs2) {
            // fcvt.s.w
            case 0b00000: f1.f = (f32)((s32)core->regs[rs1]); break;
            // fcvt.s.wu
            case 0b00001: f1.f = (f32)((u32)core->regs[rs1]); break;
            // unexpected
            default: BROADCAST(STAT_INSTR_EXCEPTION | ((u64)instr.raw << STAT_SHIFT_AMOUNT)); break;
            }
            core->fregs[rd] = f1.i;
            core->pc += 4;
            ++core->instr_analysis[FCVT2F];
            break;
        // f-mv to integer from float
        case 0b1110000:
            core->regs[rd] = core->fregs[rs1];
            core->pc += 4;
            ++core->instr_analysis[FMV2I];
            break;
        // unexpected
        default: BROADCAST(STAT_INSTR_EXCEPTION | ((u64)instr.raw << STAT_SHIFT_AMOUNT)); break;
        }
        break;
    // branch
    case 0b1100011:
        imm = instr.b.imm12 << 12 |
                instr.b.imm11 << 11 |
                instr.b.imm10_5 << 5 |
                instr.b.imm4_1 << 1;
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
        // predict branch and count stall
        core->stall_counter += core->branch_predictor->get_branch_stall(core->branch_predictor, core->pc, tmp);
        // increment pc
        core->pc += tmp ? sext(imm, 12) : 4;
        ++core->instr_analysis[BRANCH];
        break;
    // jalr
    case 0b1100111:
        imm = instr.i.imm;
        register const WORD t = core->pc + 4;
        core->pc = (core->regs[rs1] + sext(imm, 11)) & ~1;
        core->regs[rd] = t;
        // count stall
        core->stall_counter += 2;
        ++core->instr_analysis[JALR];
        break;
    // f-store
    case 0b0100111:
        imm = instr.s.imm11_5 << 5 | instr.s.imm4_0;
        core->store_data(core, core->regs[rs1] + sext(imm, 11), core->fregs[rs2]);
        core->pc += 4;
        ++core->instr_analysis[F_STORE];
        break;
    // jal
    case 0b1101111:
        imm = instr.j.imm20 << 20 |
                instr.j.imm19_12 << 12 |
                instr.j.imm11 << 11 |
                instr.j.imm10_1 << 1;
        core->regs[rd] = core->pc + 4;
        core->pc += sext(imm, 20);
        // count stall
        core->stall_counter += 2;
        ++core->instr_analysis[JAL];
        break;
    // lui
    case 0b0110111:
        imm = instr.u.imm31_12;
        core->regs[rd] = imm << 12;
        core->pc += 4;
        ++core->instr_analysis[LUI];
        break;
    // auipc
    case 0b0010111:
        imm = instr.u.imm31_12;
        core->regs[rd] = core->pc + (imm << 12);
        core->pc += 4;
        ++core->instr_analysis[AUIPC];
        break;
    // env + csr
    case 0b1110011:
        imm = instr.i.imm;
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
        ++core->instr_analysis[ENV_CSR];
        break;
    // unexpected
    default: BROADCAST(STAT_INSTR_EXCEPTION | ((u64)instr.raw << STAT_SHIFT_AMOUNT)); break;
    }
}
