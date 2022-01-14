#include "mmu.h"
#include "core.h"

WORD mmu_read_instr(const MMU* mmu, const ADDR addr) {
    return (addr < mmu->instr_len) ? mmu->instr_mem[addr] : 0;
}

void mmu_write_instr(const MMU* mmu, const ADDR addr, const WORD val) {
    if (addr < mmu->instr_len)
        mmu->instr_mem[addr] = val;
}

WORD mmu_read_data(const MMU* mmu, void* const core, const ADDR addr) {
    WORD val;
    #if defined(NO_CACHE)
    val = mmu->data_mem->read_word(mmu->data_mem, addr);
    ((CORE*)core)->stall_counter += 12;
    #else
    // cache miss
    if (!mmu->data_cache->read_word(mmu->data_cache, addr, &val)) {
        // load certain block to cache
        mmu->data_cache->load_block(mmu->data_cache, addr, mmu->data_mem);
        val = mmu->data_mem->read_word(mmu->data_mem, addr);
    }
    #endif
    return val;
}

void mmu_write_data(const MMU* mmu, void* const core, const ADDR addr, const WORD val) {
    #if defined(NO_CACHE)
    mmu->data_mem->write_word(mmu->data_mem, addr, val);
    ((CORE*)core)->stall_counter += 12;
    #else
    // cache miss
    if (!mmu->data_cache->write_word(mmu->data_cache, addr, val)) {
        // write allocate
        mmu->data_cache->load_block(mmu->data_cache, addr, mmu->data_mem);
        mmu->data_cache->write_word(mmu->data_cache, addr, val);
    }
    #endif
}

BYTE mmu_sneak(MMU* mmu, ADDR addr, u8 type) {
    WORD val;
    if (type) {
        // instr
        val = mmu->read_instr(mmu, addr >> 2);
    } else {
        // data
        #if defined(NO_CACHE)
        val = mmu->data_mem->read_word(mmu->data_mem, addr);
        #else
        if (!mmu->data_cache->sneak(mmu->data_cache, addr, &val))
            val = mmu->data_mem->read_word(mmu->data_mem, addr);
        #endif
    }
    return (val >> ((3 - (addr & 0x3)) * 8)) & 0xFF;
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
