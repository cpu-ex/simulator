#include "core.h"
#include "instr.h"

static CORE* core_base;

void if_dc_ex() {
    // fetch
    u32 raw = core_base->mmu->read_word(core_base->pc);
    // decode
    INSTR curr_instr = { .raw = raw };
    // execute
    execute(core_base, curr_instr);
    core_base->instr_counter += 1;
}

WORD load(ADDR addr, int bytes, int sign) {
    WORD val = 0;
    switch (bytes) {
    case 0: val = (WORD)core_base->mmu->read_byte(addr); break;
    case 1: val = (WORD)core_base->mmu->read_half(addr); break;
    case 2: val = (WORD)core_base->mmu->read_word(addr); break;
    default: break; // unexpected
    }
    return sign ? sext(val, (1 << bytes) * 8 - 1) : val;
}

void store(ADDR addr, WORD val, int bytes) {
    switch (bytes) {
    case 0: core_base->mmu->write_byte(addr, (BYTE)val); break;
    case 1: core_base->mmu->write_half(addr, (HALF)val); break;
    case 2: core_base->mmu->write_word(addr, (WORD)val); break;
    default: break; // unexpected
    }
}

void init_core(CORE* core, ADDR pc) {
    core_base = core;

    core->pc = pc;
    for (int i = 0; i < 32; i++)
        core->regs[i] = 0;
    
    core->instr_counter = 0;

    static MMU mmu;
    init_mmu(&mmu);
    core->mmu = &mmu;

    core->load = load;
    core->store = store;
    core->if_dc_ex = if_dc_ex;
}
