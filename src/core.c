#include "core.h"
#include "instr.h"
#include "fpu.h"

/******************** uart ********************/

u8 uart_isempty(const UART_QUEUE* uart) {
    return (uart->left < uart->right) ? 0 : 1;
}

void uart_push(UART_QUEUE* const uart, const u8 val) {
    uart->buffer[(uart->right++) % uart->size] = val;
}

u8 uart_pop(UART_QUEUE* const uart) {
    return (uart->left < uart->right) ? uart->buffer[(uart->left++) % uart->size] : 0;
}

void init_uart_queue(UART_QUEUE* uart, u32 size) {
    uart->left = 0;
    uart->right = 0;
    uart->size = size;
    uart->buffer = (u8*)malloc(size * sizeof(u8));
    uart->push = uart_push;
    uart->pop = uart_pop;
    uart->isempty = uart_isempty;
}

/******************** core lite ********************/

void core_step_lite(CORE* const core) {
    // rudely fetch
    register const INSTR instr = { .raw = core->mmu->instr_mem[core->pc >> 2] };
    // decode
    register const WORD opcode = instr.r.opcode;
    register const WORD rd = instr.r.rd;
    register const WORD rs1 = instr.r.rs1;
    register const WORD rs2 = instr.r.rs2;
    register const WORD funct3 = instr.r.funct3;
    register const WORD funct7 = instr.r.funct7;
    // execute
    register u32 imm, tmp;
    register FLOAT_HELPER f1, f2;

    switch (opcode) {
    // arith I
    case 0b0010011:
        imm = instr.i.imm;
        if (funct3) // slli
            core->regs[rd] = core->regs[rs1] << sext(imm, 11);
        else // addi
            core->regs[rd] = core->regs[rs1] + sext(imm, 11);
        core->pc += 4;
        break;
    // load
    case 0b0000011:
        imm = instr.i.imm;
        core->regs[rd] = core->load_data(core, core->regs[rs1] + sext(imm, 11));
        core->pc += 4; // ignore lw stall
        break;
    // f-arith
    case 0b1010011:
        switch (funct7) {
        // fmul
        case 0b0001000:
            core->fregs[rd] = fmul(
                (FLOAT_HELPER){ .i = core->fregs[rs1] },
                (FLOAT_HELPER){ .i = core->fregs[rs2] }
            ).i;
            core->pc += 4;
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
            break;
        // f-mv to float from integer
        case 0b1111000:
            core->fregs[rd] = core->regs[rs1];
            core->pc += 4;
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
            break;
        // fsqrt
        case 0b0101100:
            core->fregs[rd] = fsqrt((FLOAT_HELPER){ .i = core->fregs[rs1] }).i;
            core->pc += 4;
            break;
        // fdiv
        case 0b0001100:
            core->fregs[rd] = fdiv(
                (FLOAT_HELPER){ .i = core->fregs[rs1] },
                (FLOAT_HELPER){ .i = core->fregs[rs2] }
            ).i;
            core->pc += 4;
            break;
        // fcvt to integer from float
        case 0b1100000:
            f1.i = core->fregs[rs1];
            core->regs[rd] = (f1.f < 0.0) ? ((s32)(f1.f - (f32)((s32)(f1.f - 1.0)) + 0.5) + (s32)(f1.f - 1.0)) : ((s32)(f1.f + 0.5));
            core->pc += 4;
            break;
        // fcvt to float from integer
        case 0b1101000:
            f1.f = (f32)((s32)core->regs[rs1]);
            core->fregs[rd] = f1.i;
            core->pc += 4;
            break;
        // unexpected
        default: BROADCAST(STAT_INSTR_EXCEPTION | ((u64)instr.raw << STAT_SHIFT_AMOUNT)); break;
        }
        break;
    // f-load
    case 0b0000111:
        imm = instr.i.imm;
        core->fregs[rd] = core->load_data(core, core->regs[rs1] + sext(imm, 11));
        core->pc += 4; // ignore flw stall
        break;
    // store
    case 0b0100011:
        imm = instr.s.imm11_5 << 5 | instr.s.imm4_0;
        core->store_data(core, core->regs[rs1] + sext(imm, 11), core->regs[rs2]);
        core->pc += 4;
        ++core->instr_analysis[STORE];
        break;
    // vector1
    case 0b1000000:
    case 0b1000010:
        imm = instr.v1.imm;
        core->vec_addr = core->regs[rs1] + sext(imm, 11);
        core->vec_mask = instr.v1.mask;
        core->pc += 4;
        return;
    // v-load
    case 0b1100000:
        if (core->vec_mask & 0x8) core->regs[instr.v2.r2] = core->load_data(core, core->vec_addr);
        if (core->vec_mask & 0x4) core->regs[instr.v2.r3] = core->load_data(core, core->vec_addr + 0x4);
        if (core->vec_mask & 0x2) core->regs[instr.v2.r4] = core->load_data(core, core->vec_addr + 0x8);
        if (core->vec_mask & 0x1) core->regs[instr.v2.r5] = core->load_data(core, core->vec_addr + 0xC);
        core->pc += 4;
        ++core->instr_analysis[V_LOAD];
        break;
    // v-store
    case 0b1100010:
        if (core->vec_mask & 0x8) core->store_data(core, core->vec_addr, core->regs[instr.v2.r2]);
        if (core->vec_mask & 0x4) core->store_data(core, core->vec_addr + 0x4, core->regs[instr.v2.r3]);
        if (core->vec_mask & 0x2) core->store_data(core, core->vec_addr + 0x8, core->regs[instr.v2.r4]);
        if (core->vec_mask & 0x1) core->store_data(core, core->vec_addr + 0xC, core->regs[instr.v2.r5]);
        core->pc += 4;
        ++core->instr_analysis[V_STORE];
        break;
    // branch
    case 0b1100011:
        imm = instr.b.imm12 << 12 | instr.b.imm11 << 11 | instr.b.imm10_5 << 5 | instr.b.imm4_1 << 1;
        switch (funct3) {
        // beq
        case 0b000: tmp = (core->regs[rs1] == core->regs[rs2]) ? 1 : 0; break;
        // bge
        case 0b101: tmp = ((s32)core->regs[rs1] >= (s32)core->regs[rs2]) ? 1 : 0; break;
        // bne
        case 0b001: tmp = (core->regs[rs1] != core->regs[rs2]) ? 1 : 0; break;
        // blt
        case 0b100: tmp = ((s32)core->regs[rs1] < (s32)core->regs[rs2]) ? 1 : 0; break;
        // unexpected
        default: BROADCAST(STAT_INSTR_EXCEPTION | ((u64)instr.raw << STAT_SHIFT_AMOUNT)); break;
        }
        core->pc += tmp ? sext(imm, 12) : 4; // ignore branch stall
        break;
    // jal
    case 0b1101111:
        imm = instr.j.imm20 << 20 | instr.j.imm19_12 << 12 | instr.j.imm11 << 11 | instr.j.imm10_1 << 1;
        core->regs[rd] = core->pc + 4;
        core->pc += sext(imm, 20);
        break;
    // arith
    case 0b0110011:
        switch (funct3) {
        // add + sub
        case 0b000:
            if (funct7)
                core->regs[rd] = core->regs[rs1] - core->regs[rs2];
            else
                core->regs[rd] = core->regs[rs1] + core->regs[rs2];
            break;
        // sll
        case 0b001: core->regs[rd] = core->regs[rs1] << core->regs[rs2]; break;
        // or
        case 0b110: core->regs[rd] = core->regs[rs1] | core->regs[rs2]; break;
        // srl + sra
        case 0b101:
            if (funct7)
                core->regs[rd] = (u32)(((s32)core->regs[rs1]) >> core->regs[rs2]);
            else
                core->regs[rd] = core->regs[rs1] >> core->regs[rs2];
            break;
        default: BROADCAST(STAT_INSTR_EXCEPTION | ((u64)instr.raw << STAT_SHIFT_AMOUNT)); break;
        }
        core->pc += 4;
        break;
    // f-branch
    case 0b1100001:
        f1.i = core->fregs[rs1];
        f2.i = core->fregs[rs2];
        imm = instr.b.imm12 << 12 | instr.b.imm11 << 11 | instr.b.imm10_5 << 5 | instr.b.imm4_1 << 1;
        switch (funct3) {
        // bflt
        case 0b010: tmp = (f1.f < f2.f) ? 1 : 0; break;
        // bfle
        case 0b001: tmp = (f1.f <= f2.f) ? 1 : 0; break;
        // bfeq
        case 0b000: tmp = (f1.f == f2.f) ? 1 : 0; break;
        // unexpected
        default: BROADCAST(STAT_INSTR_EXCEPTION | ((u64)instr.raw << STAT_SHIFT_AMOUNT)); break;
        }
        core->pc += tmp ? sext(imm, 12) : 4; // ignore branch stall
        break;
    // f-store
    case 0b0100111:
        imm = instr.s.imm11_5 << 5 | instr.s.imm4_0;
        core->store_data(core, core->regs[rs1] + sext(imm, 11), core->fregs[rs2]);
        core->pc += 4;
        break;
    // jalr
    case 0b1100111:
        imm = instr.i.imm;
        register const WORD t = core->pc + 4;
        core->pc = core->regs[rs1] + sext(imm, 11);
        core->regs[rd] = t;
        break;
    // lui
    case 0b0110111:
        imm = instr.u.imm31_12;
        core->regs[rd] = imm << 12;
        core->pc += 4;
        break;
    // auipc
    case 0b0010111:
        imm = instr.u.imm31_12;
        core->regs[rd] = core->pc + (imm << 12);
        core->pc += 4;
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
        break;
    // unexpected
    default: BROADCAST(STAT_INSTR_EXCEPTION | ((u64)instr.raw << STAT_SHIFT_AMOUNT)); break;
    }
    // after work
    core->regs[0] = 0;
    ++core->instr_counter;
}

const WORD core_load_data_lite(const CORE* core, const ADDR addr) {
    if (addr != UART_ADDR)
        return core->mmu->data_mem->read_word(core->mmu->data_mem, addr);
    else
        return core->uart_in->pop(core->uart_in);
}

void core_store_data_lite(const CORE* core, const ADDR addr, const WORD val) {
    if (addr != UART_ADDR)
        core->mmu->data_mem->write_word(core->mmu->data_mem, addr, val);
    else
        core->uart_out->push(core->uart_out, val & 0xFF);
}

/******************** core gui ********************/

#define get_lw_stall(rd, op) ((!rd) || ((rd ^ ((op >> 15) & 0x1F)) && (rd ^ ((op >> 20) & 0x1F))) ? 0 : 1)

void core_step_gui(CORE* const core) {
    // fetch
    register const INSTR instr = { .raw = core->load_instr(core, core->pc) };
    // decode
    register const WORD opcode = instr.r.opcode;
    register const WORD rd = instr.r.rd;
    register const WORD rs1 = instr.r.rs1;
    register const WORD rs2 = instr.r.rs2;
    register const WORD funct3 = instr.r.funct3;
    register const WORD funct7 = instr.r.funct7;
    // execute
    register u32 imm, tmp;
    register FLOAT_HELPER f1, f2;

    switch (opcode) {
    // arith I
    case 0b0010011:
        imm = instr.i.imm;
        switch (funct3) {
        // addi
        case 0b000: core->regs[rd] = core->regs[rs1] + sext(imm, 11); break;
        // slli (legal when shamt[5] = 0, but not implemented)
        case 0b001: core->regs[rd] = core->regs[rs1] << sext(imm, 11); break;
        // slti (never used)
        case 0b010: core->regs[rd] = ((s32)core->regs[rs1] < (s32)sext(imm, 11)) ? 1 : 0; break;
        // xori (never used)
        case 0b100: core->regs[rd] = core->regs[rs1] ^ sext(imm, 11); break;
        // srli + srai (same with slli) (never used)
        case 0b101:
            if (imm >> 5)
                core->regs[rd] = (u32)(((s32)core->regs[rs1]) >> sext(imm, 11));
            else
                core->regs[rd] = core->regs[rs1] >> sext(imm, 11);
            break;
        // ori (never used)
        case 0b110: core->regs[rd] = core->regs[rs1] | sext(imm, 11); break;
        // andi (never used)
        case 0b111: core->regs[rd] = core->regs[rs1] & sext(imm, 11); break;
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
        // fetch next instr and count stall
        tmp = core->load_instr(core, core->pc);
        core->stall_counter += get_lw_stall(rd, tmp);
        ++core->instr_analysis[LOAD];
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
            // count stall
            core->stall_counter += 3;
            ++core->instr_analysis[FMUL];
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
            // count stall
            core->stall_counter += 3;
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
            // count stall
            core->stall_counter += 3;
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
        // fsqrt
        case 0b0101100:
            core->fregs[rd] = fsqrt((FLOAT_HELPER){ .i = core->fregs[rs1] }).i;
            core->pc += 4;
            // count stall
            core->stall_counter += 7;
            ++core->instr_analysis[FSQRT];
            break;
        // fdiv
        case 0b0001100:
            core->fregs[rd] = fdiv(
                (FLOAT_HELPER){ .i = core->fregs[rs1] },
                (FLOAT_HELPER){ .i = core->fregs[rs2] }
            ).i;
            core->pc += 4;
            // count stall
            core->stall_counter += 10;
            ++core->instr_analysis[FDIV];
            break;
        // fcvt to integer from float
        case 0b1100000:
            f1.i = core->fregs[rs1];
            // fcvt.w.s
            core->regs[rd] = (f1.f < 0.0) ? ((s32)(f1.f - (f32)((s32)(f1.f - 1.0)) + 0.5) + (s32)(f1.f - 1.0)) : ((s32)(f1.f + 0.5));
            core->pc += 4;
            // count stall
            core->stall_counter += 1;
            ++core->instr_analysis[FCVT2I];
            break;
        // fcvt to float from integer
        case 0b1101000:
            // fcvt.s.w
            f1.f = (f32)((s32)core->regs[rs1]);
            core->fregs[rd] = f1.i;
            core->pc += 4;
            // count stall
            core->stall_counter += 1;
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
    // f-load
    case 0b0000111:
        imm = instr.i.imm;
        core->fregs[rd] = core->load_data(core, core->regs[rs1] + sext(imm, 11));
        core->pc += 4;
        // fetch next instr and count stall
        tmp = core->load_instr(core, core->pc);
        core->stall_counter += get_lw_stall(rd, tmp);
        ++core->instr_analysis[F_LOAD];
        break;
    // store
    case 0b0100011:
        imm = instr.s.imm11_5 << 5 | instr.s.imm4_0;
        core->store_data(core, core->regs[rs1] + sext(imm, 11), core->regs[rs2]);
        core->pc += 4;
        ++core->instr_analysis[STORE];
        break;
    // vector1
    case 0b1000000:
    case 0b1000010:
        imm = instr.v1.imm;
        core->vec_addr = core->regs[rs1] + sext(imm, 11);
        core->vec_mask = instr.v1.mask;
        core->pc += 4;
        return;
    // v-load
    case 0b1100000:
        if (core->vec_mask & 0x8) core->regs[instr.v2.r2] = core->load_data(core, core->vec_addr);
        if (core->vec_mask & 0x4) core->regs[instr.v2.r3] = core->load_data(core, core->vec_addr + 0x4);
        if (core->vec_mask & 0x2) core->regs[instr.v2.r4] = core->load_data(core, core->vec_addr + 0x8);
        if (core->vec_mask & 0x1) core->regs[instr.v2.r5] = core->load_data(core, core->vec_addr + 0xC);
        core->pc += 4;
        ++core->instr_analysis[V_LOAD];
        break;
    // v-store
    case 0b1100010:
        if (core->vec_mask & 0x8) core->store_data(core, core->vec_addr, core->regs[instr.v2.r2]);
        if (core->vec_mask & 0x4) core->store_data(core, core->vec_addr + 0x4, core->regs[instr.v2.r3]);
        if (core->vec_mask & 0x2) core->store_data(core, core->vec_addr + 0x8, core->regs[instr.v2.r4]);
        if (core->vec_mask & 0x1) core->store_data(core, core->vec_addr + 0xC, core->regs[instr.v2.r5]);
        core->pc += 4;
        ++core->instr_analysis[V_STORE];
        break;
    // branch
    case 0b1100011:
        imm = instr.b.imm12 << 12 | instr.b.imm11 << 11 | instr.b.imm10_5 << 5 | instr.b.imm4_1 << 1;
        switch (funct3) {
        // beq
        case 0b000: tmp = (core->regs[rs1] == core->regs[rs2]) ? 1 : 0; break;
        // bge
        case 0b101: tmp = ((s32)core->regs[rs1] >= (s32)core->regs[rs2]) ? 1 : 0; break;
        // bne
        case 0b001: tmp = (core->regs[rs1] != core->regs[rs2]) ? 1 : 0; break;
        // blt
        case 0b100: tmp = ((s32)core->regs[rs1] < (s32)core->regs[rs2]) ? 1 : 0; break;
        // unexpected
        default: BROADCAST(STAT_INSTR_EXCEPTION | ((u64)instr.raw << STAT_SHIFT_AMOUNT)); break;
        }
        // predict branch and count stall
        core->stall_counter += core->branch_predictor->get_branch_stall(core->branch_predictor, core->pc, tmp);
        // increment pc
        core->pc += tmp ? sext(imm, 12) : 4;
        ++core->instr_analysis[BRANCH];
        break;
    // jal
    case 0b1101111:
        imm = instr.j.imm20 << 20 | instr.j.imm19_12 << 12 | instr.j.imm11 << 11 | instr.j.imm10_1 << 1;
        core->regs[rd] = core->pc + 4;
        core->pc += sext(imm, 20);
        ++core->instr_analysis[JAL];
        break;
    // arith
    case 0b0110011:
        switch (funct3) {
        // add + sub
        case 0b000:
            if (funct7)
                core->regs[rd] = core->regs[rs1] - core->regs[rs2];
            else
                core->regs[rd] = core->regs[rs1] + core->regs[rs2];
            break;
        // sll
        case 0b001: core->regs[rd] = core->regs[rs1] << core->regs[rs2]; break;
        // or
        case 0b110: core->regs[rd] = core->regs[rs1] | core->regs[rs2]; break;
        // srl + sra
        case 0b101:
            if (funct7)
                core->regs[rd] = (u32)(((s32)core->regs[rs1]) >> core->regs[rs2]);
            else
                core->regs[rd] = core->regs[rs1] >> core->regs[rs2];
            break;
        // slt (never used)
        case 0b010: core->regs[rd] = ((s32)core->regs[rs1] < (s32)core->regs[rs2]) ? 1 : 0; break;
        // xor (never used)
        case 0b100: core->regs[rd] = core->regs[rs1] ^ core->regs[rs2]; break;
        // and (never used)
        case 0b111: core->regs[rd] = core->regs[rs1] & core->regs[rs2]; break;
        // unexpected
        default: BROADCAST(STAT_INSTR_EXCEPTION | ((u64)instr.raw << STAT_SHIFT_AMOUNT)); break;
        }
        core->pc += 4;
        ++core->instr_analysis[ARITH];
        break;
    // f-branch
    case 0b1100001:
        f1.i = core->fregs[rs1];
        f2.i = core->fregs[rs2];
        imm = instr.b.imm12 << 12 | instr.b.imm11 << 11 | instr.b.imm10_5 << 5 | instr.b.imm4_1 << 1;
        switch (funct3) {
        // bflt
        case 0b010: tmp = (f1.f < f2.f) ? 1 : 0; break;
        // bfle
        case 0b001: tmp = (f1.f <= f2.f) ? 1 : 0; break;
        // bfeq
        case 0b000: tmp = (f1.f == f2.f) ? 1 : 0; break;
        // unexpected
        default: BROADCAST(STAT_INSTR_EXCEPTION | ((u64)instr.raw << STAT_SHIFT_AMOUNT)); break;
        }
        // predict branch and count stall
        core->stall_counter += core->branch_predictor->get_branch_stall(core->branch_predictor, core->pc, tmp);
        // increment pc
        core->pc += tmp ? sext(imm, 12) : 4;
        ++core->instr_analysis[F_BRANCH];
        break;
    // f-store
    case 0b0100111:
        imm = instr.s.imm11_5 << 5 | instr.s.imm4_0;
        core->store_data(core, core->regs[rs1] + sext(imm, 11), core->fregs[rs2]);
        core->pc += 4;
        ++core->instr_analysis[F_STORE];
        break;
    // jalr
    case 0b1100111:
        imm = instr.i.imm;
        register const WORD t = core->pc + 4;
        core->pc = core->regs[rs1] + sext(imm, 11);
        core->regs[rd] = t;
        // count stall
        core->stall_counter += 2;
        ++core->instr_analysis[JALR];
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
    // after work
    core->regs[0] = 0;
    ++core->instr_counter;
}

const WORD core_load_data_gui(const CORE* core, const ADDR addr) {
    if (addr != UART_ADDR)
        return core->mmu->read_data(core->mmu, (void* const)core, addr);
    else
        return core->uart_in->pop(core->uart_in);
}

void core_store_data_gui(const CORE* core, const ADDR addr, const WORD val) {
    if (addr != UART_ADDR)
        core->mmu->write_data(core->mmu, (void* const)core, addr, val);
    else
        core->uart_out->push(core->uart_out, val & 0xFF);
}

/******************** core common attributes ********************/

const WORD core_load_instr(const CORE* core, const ADDR addr) {
    return core->mmu->read_instr(core->mmu, addr >> 2);
}

void core_store_instr(const CORE* core, const ADDR addr, const WORD val) {
    core->mmu->write_instr(core->mmu, addr >> 2, val);
}

void core_dump(CORE* core) {
    // check dumpfile
    if (core->dumpfile_fp == NULL)
        core->dumpfile_fp = fopen(core->dumpfile_name, "w");
    // step, pc
    fprintf(core->dumpfile_fp, "step:%016llx pc:%08x", core->instr_counter, core->pc); 
    // register file
    for (int i = 0; i < 64; ++i) {
        if (i < 32) {
            fprintf(core->dumpfile_fp, " x%d:%08x", i, core->regs[i]);
        } else {
            fprintf(core->dumpfile_fp, " f%d:%08x", i - 32, core->fregs[i - 32]);
        }
    }
    fprintf(core->dumpfile_fp, "\r\n");
}

void core_reset(CORE* core) {
    // call mem reset
    core->mmu->reset(core->mmu, core->regs[2]);
    // reset registers
    core->pc = DEFAULT_PC;
    memset(core->regs, 0, 32 * sizeof(u32));
    memset(core->fregs, 0, 32 * sizeof(u32));
    // reset uart queue
    core->uart_in->left = 0;
    core->uart_out->right = 0;
    // reset instruction analysis
    core->instr_counter = 0;
    core->stall_counter = 0;
    memset(core->instr_analysis, 0, 26 * sizeof(u64));
    // reset branch predictor
    core->branch_predictor->reset(core->branch_predictor);
}

#define close_file(fp, name)             \
    {                                    \
        if (fp != NULL) {                \
            fseek(fp, 0, SEEK_END);      \
            u64 filesize = ftell(fp);    \
            fclose(fp);                  \
            if (!filesize) remove(name); \
            fp = NULL;                   \
        }                                \
    }
void core_deinit(CORE* core) {
    FILE* outputfile_fp = fopen(core->outputfile_name, "w");
    while (!core->uart_out->isempty(core->uart_out)) {
        u8 byte = core->uart_out->pop(core->uart_out);
        fwrite(&byte, 1, 1, outputfile_fp);
    }
    close_file(outputfile_fp, core->outputfile_name);
    close_file(core->dumpfile_fp, core->dumpfile_name);
}

f64 core_predict_exec_time(CORE* core) {
    f64 code_sending = (f64)core->mmu->instr_len * 40.0 / UART_BAUDRATE;
    f64 instr_executing = (f64)(core->instr_counter + core->stall_counter) / CLK_FREQUENCY;
    return code_sending + instr_executing;
}

void init_core(CORE* core, u8 is_lite) {
    // init basic info
    core->pc = DEFAULT_PC;
    core->instr_counter = 0;
    core->stall_counter = 0;
    memset(core->instr_analysis, 0, 26 * sizeof(u64));
    // prepare for outputs
    time_t curr_time = time(NULL);
    struct tm* info = localtime(&curr_time);
    strftime(core->outputfile_name, 30, "output-%Y%m%d-%H%M%S", info);
    strftime(core->dumpfile_name, 30, "dumpfile-%Y%m%d-%H%M%S", info);
    // init fpu
    init_fpu();
    // assign interfaces
    core->load_instr = core_load_instr;
    core->load_data = is_lite ? core_load_data_lite : core_load_data_gui;
    core->store_instr = core_store_instr;
    core->store_data = is_lite ? core_store_data_lite : core_store_data_gui;
    core->step = is_lite ? core_step_lite : core_step_gui;
    core->dump = core_dump;
    core->reset = core_reset;
    core->deinit = core_deinit;
    core->predict_exec_time = core_predict_exec_time;
}
