#include "mmu.h"

WORD mmu_read_instr(MMU* mmu, ADDR addr) {
    return (addr < mmu->instr_len) ? mmu->instr_mem[addr] : 0;
}

void mmu_write_instr(MMU* mmu, ADDR addr, WORD val) {
    if (addr < mmu->instr_len)
        mmu->instr_mem[addr] = val;
}

BYTE mmu_read_data(MMU* mmu, ADDR addr) {
    BYTE val;
    // cache miss
    if (!mmu->data_cache->read_byte(mmu->data_cache, addr, &val)) {
        // load certain block to cache
        mmu->data_cache->load_block(mmu->data_cache, addr, mmu->data_mem);
        val = mmu->data_mem->read_byte(mmu->data_mem, addr);
    }
    return val;
}

void mmu_write_data(MMU* mmu, ADDR addr, BYTE val) {
    // cache miss
    if (!mmu->data_cache->write_byte(mmu->data_cache, addr, val)) {
        // write allocate
        mmu->data_cache->load_block(mmu->data_cache, addr, mmu->data_mem);
        mmu->data_cache->write_byte(mmu->data_cache, addr, val);
    }
}

BYTE mmu_sneak(MMU* mmu, ADDR addr, u8 type) {
    if (type) {
        // instr
        WORD val = mmu->read_instr(mmu, addr >> 2);
        return (val >> ((3 - (addr & 0x3)) * 8)) & 0xFF;
    } else {
        // data
        BYTE val;
        if (!mmu->data_cache->sneak(mmu->data_cache, addr, &val))
            val = mmu->data_mem->read_byte(mmu->data_mem, addr);
        return val;
    }
}

void mmu_allocate_instr(MMU* mmu, u64 size) {
    mmu->instr_len = size;
    mmu->instr_mem = malloc(size * sizeof(WORD));
}

void mmu_reset(MMU* mmu, ADDR addr) {
    mmu->data_cache->reset(mmu->data_cache);
    mmu->data_mem->reset_stack(mmu->data_mem, addr);
}

void init_mmu(MMU* mmu) {
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
    mmu->sneak = mmu_sneak;
    mmu->reset = mmu_reset;
}
