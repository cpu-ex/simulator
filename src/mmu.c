#include "mmu.h"

static MMU* mmu_base;

BYTE mmu_read_instr(ADDR addr) {
    if (addr < mmu_base->instr_len) {
        return mmu_base->instr_mem[addr];
    } else {
        BROADCAST(STAT_MEM_EXCEPTION | ((u64)addr << 32));
        return 0;
    }
}

void mmu_write_instr(ADDR addr, BYTE val) {
    if (addr < mmu_base->instr_len) {
        mmu_base->instr_mem[addr] = val;
    } else {
        BROADCAST(STAT_MEM_EXCEPTION | ((u64)addr << 32));
    }
}

void allocate_instr(u64 size) {
    mmu_base->instr_len = size;
    mmu_base->instr_mem = malloc(size * sizeof(BYTE));
}

void init_mmu(MMU* mmu) {
    mmu_base = mmu;
    static MEM data_mem;
    init_mem(&data_mem);
    mmu->data_mem = &data_mem;
    // assign interfaces
    mmu->allocate_instr = allocate_instr;
    mmu->read_instr = mmu_read_instr;
    mmu->write_instr = mmu_write_instr;
    mmu->read_data = mmu->data_mem->read_byte;
    mmu->write_data = mmu->data_mem->write_byte;
    mmu->reset = mmu->data_mem->reset_stack;
}
