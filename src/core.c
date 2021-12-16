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
    // step, pc
    fprintf(core->dumpfile_fp, "step:%016llx pc:%08x", core->instr_counter, core->pc); 
    // register file
    for (int i = 0; i < 64; i++) {
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

void core_deinit(CORE* core) {
    fseek(core->dumpfile_fp, 0, SEEK_END);
    u64 filesize = ftell(core->dumpfile_fp);
    fclose(core->dumpfile_fp);
    if (!filesize) remove(core->dumpfile_name);
    core->dumpfile_fp = NULL;
}

void init_core(CORE* core) {
    // init basic info
    core->pc = DEFAULT_PC;
    core->instr_counter = 0;
    core->stall_counter = 0;
    memset(core->instr_analysis, 0, 23 * sizeof(u32));
    // open a dumpfile
    time_t curr_time = time(NULL);
    struct tm* info = localtime(&curr_time);
    strftime(core->dumpfile_name, 30, "dumpfile-%Y%m%d-%H%M%S.txt", info);
    core->dumpfile_fp = fopen(core->dumpfile_name, "w");
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
    core->deinit = core_deinit;
}
