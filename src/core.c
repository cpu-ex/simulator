#include "core.h"
#include "exec.h"

void core_step(CORE* core) {
    // fetch
    u32 raw = core->load_instr(core, core->pc);
    // decode
    INSTR curr_instr = { .raw = raw };
    // execute
    execute(core, curr_instr);
    core->regs[zero] = 0;
    core->instr_counter += 1;
}

WORD core_load_instr(CORE* core, ADDR addr) {
    addr &= ~0x3;
    WORD val = 0;
    for (int i = 0; i < 4; i++) {
        val <<= 8;
        val |= core->mmu->read_instr(core->mmu, addr + i);
    }
    return val;
}

WORD core_load_data(CORE* core, ADDR addr, int bytes, int sign) {
    WORD val = 0;
    switch (addr & (~0x3)) {
    case UART_IN: val = core->uart_in; break;
    case UART_IN_VALID:
        val = core->uart_in_valid;
        core->uart_in_valid = 0;
        break;
    case UART_OUT_VALID:
        val = core->uart_out_valid;
        core->uart_out_valid = 0;
        break;
    case UART_OUT: val = core->uart_out; break;
    default:
        for (int i = 0; i < (1 << bytes); i++) {
            val <<= 8;
            val |= core->mmu->read_data(core->mmu, addr + i);
        }
        break;
    }
    return sign ? sext(val, (1 << bytes) * 8 - 1) : val;
}

void core_store_instr(CORE* core, ADDR addr, WORD val) {
    addr &= ~0x3;
    for (int i = 3; i >= 0; i--) {
        core->mmu->write_instr(core->mmu, addr + i, val & 0xFF);
        val >>= 8;
    }
}

void core_store_data(CORE* core, ADDR addr, WORD val, int bytes) {
    switch (addr & (~0x3)) {
    case UART_IN: core->uart_in = val; break;
    case UART_IN_VALID: core->uart_in_valid = val; break;
    case UART_OUT_VALID: core->uart_out_valid = val; break;
    case UART_OUT: core->uart_out = val; break;
    default:
        for (int i = (1 << bytes) - 1; i >= 0; i--) {
            core->mmu->write_data(core->mmu, addr + i, val & 0xFF);
            val >>= 8;
        }
        break;
    }
}

void core_reset(CORE* core) {
    // call mem reset
    core->mmu->reset(core->mmu, core->regs[sp]);
    // reset registers
    core->pc = DEFAULT_PC;
    for (int i = 0; i < 32; i++) {
        core->regs[i] = 0;
        core->fregs[i] = 0;
    }
    // reset instruction analysis
    core->instr_counter = 0;
    memset(core->instr_analysis, 0, 23 * sizeof(u32));
    // reset branch predictor
    core->branch_predictor->reset(core->branch_predictor);
}

void init_core(CORE* core) {
    // init basic info
    core->pc = DEFAULT_PC;
    core->instr_counter = 0;
    memset(core->instr_analysis, 0, 23 * sizeof(u32));
    // init mmu
    static MMU mmu;
    init_mmu(&mmu);
    core->mmu = &mmu;
    // init branch predictor
    static BRANCH_PREDICTOR branch_predictor;
    init_branch_predictor(&branch_predictor);
    core->branch_predictor = &branch_predictor;
    // assign interfaces
    core->load_instr = core_load_instr;
    core->load_data = core_load_data;
    core->store_instr = core_store_instr;
    core->store_data = core_store_data;
    core->step = core_step;
    core->reset = core_reset;
}
