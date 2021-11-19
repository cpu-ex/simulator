#include "core.h"
#include "exec.h"

static CORE* core_base;

void core_step() {
    // fetch
    u32 raw = core_base->load_instr(core_base->pc);
    // decode
    INSTR curr_instr = { .raw = raw };
    // execute
    execute(core_base, curr_instr);
    core_base->regs[zero] = 0;
    core_base->instr_counter += 1;
}

WORD core_load_instr(ADDR addr) {
    addr &= ~0x3;
    WORD val = 0;
    for (int i = 0; i < 4; i++) {
        val <<= 8;
        val |= core_base->mmu->read_instr(addr + i);
    }
    return val;
}

WORD core_load_data(ADDR addr, int bytes, int sign) {
    WORD val = 0;
    switch (addr & (~0x3)) {
    case UART_IN: val = core_base->uart_in; break;
    case UART_IN_VALID:
        val = core_base->uart_in_valid;
        core_base->uart_in_valid = 0;
        break;
    case UART_OUT_VALID:
        val = core_base->uart_out_valid;
        core_base->uart_out_valid = 0;
        break;
    case UART_OUT: val = core_base->uart_out; break;
    default:
        for (int i = 0; i < (1 << bytes); i++) {
            val <<= 8;
            val |= core_base->mmu->read_data(addr + i);
        }
        break;
    }
    return sign ? sext(val, (1 << bytes) * 8 - 1) : val;
}

void core_store_instr(ADDR addr, WORD val) {
    addr &= ~0x3;
    for (int i = 3; i >= 0; i--) {
        core_base->mmu->write_instr(addr + i, val & 0xFF);
        val >>= 8;
    }
}

void core_store_data(ADDR addr, WORD val, int bytes) {
    switch (addr & (~0x3)) {
    case UART_IN: core_base->uart_in = val; break;
    case UART_IN_VALID: core_base->uart_in_valid = val; break;
    case UART_OUT_VALID: core_base->uart_out_valid = val; break;
    case UART_OUT: core_base->uart_out = val; break;
    default:
        for (int i = (1 << bytes) - 1; i >= 0; i--) {
            core_base->mmu->write_data(addr + i, val & 0xFF);
            val >>= 8;
        }
        break;
    }
}

void core_reset() {
    // call mem reset
    core_base->mmu->reset(core_base->regs[sp]);
    // reset registers
    core_base->pc = DEFAULT_PC;
    for (int i = 0; i < 32; i++)
        core_base->regs[i] = 0;
    // reset instruction analysis
    core_base->instr_counter = 0;
    memset(core_base->instr_analysis, 0, 10 * sizeof(u32));
}

void init_core(CORE* core) {
    core_base = core;
    // init basic info
    core->pc = DEFAULT_PC;
    core->instr_counter = 0;
    memset(core->instr_analysis, 0, 23 * sizeof(u32));
    // init mmu
    static MMU mmu;
    init_mmu(&mmu);
    core->mmu = &mmu;
    // assign interfaces
    core->load_instr = core_load_instr;
    core->load_data = core_load_data;
    core->store_instr = core_store_instr;
    core->store_data = core_store_data;
    core->step = core_step;
    core->reset = core_reset;
}
