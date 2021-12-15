#include "core.h"
#include "exec.h"

void core_step(CORE* core) {
    // fetch
    u32 raw = core->load_instr(core, core->pc);
    // decode
    INSTR curr_instr = { .raw = raw };
    // execute
    execute(core, curr_instr);
    core->regs[0] = 0;
    core->instr_counter++;
}

WORD core_load_instr(CORE* core, ADDR addr) {
    return core->mmu->read_instr(core->mmu, addr >> 2);
}

WORD core_load_data(CORE* core, ADDR addr, u8 bytes, u8 sign) {
    WORD val = 0;
    for (int i = 0; i < (1 << bytes); i++) {
        val <<= 8;
        val |= core->mmu->read_data(core->mmu, addr + i);
    }
    return sign ? sext(val, (1 << bytes) * 8 - 1) : val;
}

void core_store_instr(CORE* core, ADDR addr, WORD val) {
    core->mmu->write_instr(core->mmu, addr & (~0x3), val);
}

void core_store_data(CORE* core, ADDR addr, WORD val, u8 bytes) {
    for (int i = (1 << bytes) - 1; i >= 0; i--) {
        core->mmu->write_data(core->mmu, addr + i, val & 0xFF);
        val >>= 8;
    }
}

void core_dump(CORE* core, s64 step_left) {
    if (core->output_file == NULL)
        core->output_file = fopen("output.txt", "a");
    fprintf(core->output_file, "PC = %08X\n", core->pc);
    if (step_left == 0) {
        fclose(core->output_file);
        core->output_file = NULL;
    }
}

void core_reset(CORE* core) {
    // call mem reset
    core->mmu->reset(core->mmu, core->regs[2]);
    // reset registers
    core->pc = DEFAULT_PC;
    for (int i = 0; i < 32; i++) {
        core->regs[i] = 0;
        core->fregs[i] = 0;
    }
    // reset instruction analysis
    core->instr_counter = 0;
    core->stall_counter = 0;
    memset(core->instr_analysis, 0, 23 * sizeof(u32));
    // reset branch predictor
    core->branch_predictor->reset(core->branch_predictor);
}

void init_core(CORE* core) {
    // init basic info
    core->pc = DEFAULT_PC;
    core->instr_counter = 0;
    core->stall_counter = 0;
    memset(core->instr_analysis, 0, 23 * sizeof(u32));
    core->output_file = NULL;
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
    core->dump = core_dump;
    core->reset = core_reset;
}
