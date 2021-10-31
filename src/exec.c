#include "exec.h"

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
    int sign = ~(funct3 & 0b100);
    core->regs[rd] = core->load(core->regs[rs1] + sext(imm, 11), bytes, sign);
    core->pc += 4;
}

// store: sb, sh, sw
void STORE_EXEC(CORE* core, INSTR instr) {
    WORD imm = instr.s.imm11_5 << 5 |
                instr.s.imm4_0;
    BYTE rs1 = instr.s.rs1;
    BYTE rs2 = instr.s.rs2;
    BYTE funct3 = instr.s.funct3;

    // funct3: 000 -> sb, 001 -> sh, 010 -> sw
    core->store(core->regs[rs1] + sext(imm, 11), core->regs[rs2], funct3);
    core->pc += 4;
}

#define ARITH_ADD(a1, a2, funct7) ((funct7) ? ((a1) - (a2)) : ((a1) + (a2)))
#define ARITH_SLL(a1, a2) ((a1) << ((a2) & 0b11111)) // only take lower 5 bits as shift amount
#define ARITH_SLT(a1, a2) (((signed)(a1) < (signed)(a2)) ? 1 : 0)
#define ARITH_SLTU(a1, a2) (((a1) < (a2)) ? 1 : 0)
#define ARITH_XOR(a1, a2) ((a1) ^ (a2))
#define ARITH_SRL(a1, a2) ((a1) >> ((a2) & 0b11111)) // same with sll
#define ARITH_SRA(a1, a2) (unsigned)(((signed)(a1)) >> ((a2)&0b11111)) // same with sll
#define ARITH_SR(a1, a2, funct7) ((funct7) ? ARITH_SRA((a1), (a2)) : ARITH_SRL((a1), (a2)))
#define ARITH_OR(a1, a2) ((a1) | (a2))
#define ARITH_AND(a1, a2) ((a1) & (a2))

// arith with immediate variants
// addi, slli, slti, sltiu, xori, srli, srai, ori, andi
void ARITH_I_EXEC(CORE *core, INSTR instr) {
    BYTE rd = instr.i.rd;
    WORD imm = instr.i.imm;
    BYTE rs1 = instr.i.rs1;
    BYTE funct3 = instr.i.funct3;

    WORD val = 0;
    WORD a1 = core->regs[rs1], a2 = sext(imm, 11);
    switch (funct3) {
    // addi
    case 0b000: val = ARITH_ADD(a1, a2, 0); break;
    // slli (legal when shamt[5] = 0, but not implemented)
    case 0b001: val = ARITH_SLL(a1, a2); break;
    // slti
    case 0b010: val = ARITH_SLT(a1, a2); break;
    // sltiu
    case 0b011: val = ARITH_SLTU(a1, a2); break;
    // xori
    case 0b100: val = ARITH_XOR(a1, a2); break;
    // srli & srai (same with slli)
    case 0b101: val = ARITH_SR(a1, a2, a2 & 0xFE0); break;
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

// arith: add, sub, sll, slt, sltu, xor, srl, sra, or, and
void ARITH_EXEC(CORE *core, INSTR instr) {
    BYTE rd = instr.r.rd;
    BYTE rs1 = instr.r.rs1;
    BYTE rs2 = instr.r.rs2;
    BYTE funct3 = instr.r.funct3;
    BYTE funct7 = instr.r.funct7;

    WORD val = 0;
    WORD a1 = core->regs[rs1], a2 = core->regs[rs2];
    switch (funct3) {
    // add & sub
    case 0b000: val = ARITH_ADD(a1, a2, funct7); break;
    // sll
    case 0b001: val = ARITH_SLL(a1, a2); break;
    // slt
    case 0b010: val = ARITH_SLT(a1, a2); break;
    // sltu
    case 0b011: val = ARITH_SLTU(a1, a2); break;
    // xor
    case 0b100: val = ARITH_XOR(a1, a2); break;
    // srl & sra
    case 0b101: val = ARITH_SR(a1, a2, funct7); break;
    // or
    case 0b110: val = ARITH_OR(a1, a2); break;
    // and
    case 0b111: val = ARITH_AND(a1, a2); break;
    // unexpected
    default: BROADCAST(STAT_INSTR_EXCEPTION | ((u64)instr.raw << 32)); break;
    }
    core->regs[rd] = val;
    core->pc += 4;
}

// env: ebreak
void ENV_EXEC(CORE *core, INSTR instr) {
    WORD imm = instr.i.imm;
    BYTE funct3 = instr.i.funct3;

    if ((imm == 1) && (funct3 == 0)) {
        // ebreak
        BROADCAST(STAT_EXIT);
    } else {
        // not implemented
        BROADCAST(STAT_INSTR_EXCEPTION | ((u64)instr.raw << 32));
    }
}

void execute(CORE* core, INSTR instr) {
    switch (instr.decoder.opcode) {
    /* risc-v I */
    // lui
    case 0b0110111: LUI_EXEC(core, instr); break;
    // auipc
    case 0b0010111: AUIPC_EXEC(core, instr); break;
    // jal
    case 0b1101111: JAL_EXEC(core, instr); break;
    // jalr
    case 0b1100111: JALR_EXEC(core, instr); break;
    // branch
    case 0b1100011: BRANCH_EXEC(core, instr); break;
    // load
    case 0b0000011: LOAD_EXEC(core, instr); break;
    // store
    case 0b0100011: STORE_EXEC(core, instr); break;
    // arith I
    case 0b0010011: ARITH_I_EXEC(core, instr); break;
    // arith
    case 0b0110011: ARITH_EXEC(core, instr); break;
    // fence
    case 0b0001111:
    // env + csr
    case 0b1110011: ENV_EXEC(core, instr); break;
    // unexpected
    default: core->pc += 4; break;
    }
}
