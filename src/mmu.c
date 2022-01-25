#include "mmu.h"
#include "core.h"

const WORD mmu_read_instr(const MMU* mmu, const ADDR addr) {
    return (addr < mmu->instr_len) ? mmu->instr_mem[addr] : 0;
}

const WORD mmu_read_data_cached(const MMU* mmu, void* const core, const ADDR addr) {
    WORD val;
    // cache miss
    if (mmu->data_cache->read_word(mmu->data_cache, addr, &val)) {
        // count stall
        // ((CORE*)core)->stall_counter += TODO;
        // load certain block to cache
        mmu->data_cache->load_block(mmu->data_cache, addr, mmu->data_mem);
        val = mmu->data_mem->read_word(mmu->data_mem, addr);
    }
    return val;
}

const WORD mmu_read_data_nocache(const MMU* mmu, void* const core, const ADDR addr) {
    // count stall
    ((CORE*)core)->stall_counter += 50;
    return mmu->data_mem->read_word(mmu->data_mem, addr);
}

void mmu_write_instr(const MMU* mmu, const ADDR addr, const WORD val) {
    if (addr < mmu->instr_len)
        mmu->instr_mem[addr] = val;
}

void mmu_write_data_cached(const MMU* mmu, void* const core, const ADDR addr, const WORD val) {
    // cache miss
    if (mmu->data_cache->write_word(mmu->data_cache, addr, val)) {
        // count stall
        // ((CORE*)core)->stall_counter += TODO;
        // write allocate
        mmu->data_cache->load_block(mmu->data_cache, addr, mmu->data_mem);
        mmu->data_cache->write_word(mmu->data_cache, addr, val);
    }
}

void mmu_write_data_nocache(const MMU* mmu, void* const core, const ADDR addr, const WORD val) {
    // count stall
    ((CORE*)core)->stall_counter += 26;
    mmu->data_mem->write_word(mmu->data_mem, addr, val);
}

BYTE mmu_sneak_cached(MMU* mmu, ADDR addr, u8 type) {
    WORD val;
    if (type) {
        // instr
        val = mmu->read_instr(mmu, addr >> 2);
    } else {
        // data
        if (mmu->data_cache->sneak(mmu->data_cache, addr, &val))
            val = mmu->data_mem->read_word(mmu->data_mem, addr);
    }
    return (val >> ((3 - (addr & 0x3)) * 8)) & 0xFF;
}

BYTE mmu_sneak_nocache(MMU* mmu, ADDR addr, u8 type) {
    WORD val;
    if (type) {
        // instr
        val = mmu->read_instr(mmu, addr >> 2);
    } else {
        // data
        val = mmu->data_mem->read_word(mmu->data_mem, addr);
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

void init_mmu(MMU* mmu, u8 is_nocache) {
    mmu->is_nocache = is_nocache;
    // init cache
    if (!is_nocache) {
        static CACHE data_cache;
        init_cache(&data_cache);
        mmu->data_cache = &data_cache;
    }
    // init memory
    static MEM data_mem;
    init_mem(&data_mem);
    mmu->data_mem = &data_mem;
    // assign interfaces
    mmu->allocate_instr = mmu_allocate_instr;
    mmu->read_instr = mmu_read_instr;
    mmu->write_instr = mmu_write_instr;
    mmu->read_data = is_nocache ? mmu_read_data_nocache : mmu_read_data_cached;
    mmu->write_data = is_nocache ? mmu_write_data_nocache : mmu_write_data_cached;
    mmu->sneak = is_nocache ? mmu_sneak_nocache : mmu_sneak_cached;
    mmu->reset = mmu_reset;
}
