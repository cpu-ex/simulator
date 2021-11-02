#include "core.h"
#include "exec.h"

static CORE* core_base;

#define DEFAULT_PC 0x10000

void step() {
    // fetch
    u32 raw = core_base->mmu->read_word(core_base->pc);
    // decode
    INSTR curr_instr = { .raw = raw };
    // execute
    execute(core_base, curr_instr);
    core_base->regs[zero] = 0;
    core_base->instr_counter += 1;
}

WORD load(ADDR addr, int bytes, int sign) {
    WORD val = 0;
    switch (bytes) {
    case 0: val = (WORD)core_base->mmu->read_byte(addr); break;
    case 1: val = (WORD)core_base->mmu->read_half(addr); break;
    case 2: val = (WORD)core_base->mmu->read_word(addr); break;
    default: BROADCAST(STAT_MEM_EXCEPTION | ((u64)addr << 32)); break; // unexpected
    }
    return sign ? sext(val, (1 << bytes) * 8 - 1) : val;
}

void store(ADDR addr, WORD val, int bytes) {
    switch (bytes) {
    case 0: core_base->mmu->write_byte(addr, (BYTE)val); break;
    case 1: core_base->mmu->write_half(addr, (HALF)val); break;
    case 2: core_base->mmu->write_word(addr, (WORD)val); break;
    default: BROADCAST(STAT_MEM_EXCEPTION | ((u64)addr << 32)); break; // unexpected
    }
}

void core_reset() {
    // reset registers
    core_base->pc = DEFAULT_PC;
    for (int i = 0; i < 32; i++)
        core_base->regs[i] = 0;
    // reset instruction analysis
    core_base->instr_counter = 0;
    memset(core_base->instr_analysis, 0, 10 * sizeof(u32));
    // call mem reset
    core_base->mmu->reset();
}

void init_core(CORE* core) {
    core_base = core;
    // init basic info
    core->pc = DEFAULT_PC;
    core->instr_counter = 0;
    memset(core->instr_analysis, 0, 10 * sizeof(u32));
    // init mmu
    static MMU mmu;
    init_mmu(&mmu);
    core->mmu = &mmu;
    // assign interfaces
    core->load = load;
    core->store = store;
    core->step = step;
    core->reset = core_reset;
}
