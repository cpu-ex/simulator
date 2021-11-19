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

BYTE mmu_read_data(ADDR addr) {
    BYTE val;
    // cache miss
    if (!mmu_base->data_cache->read_byte(addr, &val)) {
        // load certain block to cache
        mmu_base->data_cache->load_block(addr, mmu_base->data_mem->read_byte,
            mmu_base->data_mem->write_byte);
        val = mmu_base->data_mem->read_byte(addr);
    }
    return val;
}

void mmu_write_data(ADDR addr, BYTE val) {
    // cache miss
    if (!mmu_base->data_cache->write_byte(addr, val)) {
        // write allocate
        mmu_base->data_cache->load_block(addr, mmu_base->data_mem->read_byte,
            mmu_base->data_mem->write_byte);
        mmu_base->data_cache->write_byte(addr, val);
    }
}

void mmu_allocate_instr(u64 size) {
    mmu_base->instr_len = size;
    mmu_base->instr_mem = malloc(size * sizeof(BYTE));
}

void mmu_reset(ADDR addr) {
    mmu_base->data_cache->reset();
    mmu_base->data_mem->reset_stack(addr);
}

void init_mmu(MMU* mmu) {
    mmu_base = mmu;
    // init cache
    static CACHE data_cache;
    init_cache(&data_cache);
    mmu->data_cache = &data_cache;
    // init memory
    static MEM data_mem;
    init_mem(&data_mem);
    mmu->data_mem = &data_mem;
    // assign interfaces
    mmu->allocate_instr = mmu_allocate_instr;
    mmu->read_instr = mmu_read_instr;
    mmu->write_instr = mmu_write_instr;
    mmu->read_data = mmu_read_data;
    mmu->write_data = mmu_write_data;
    mmu->reset = mmu_reset;
}
