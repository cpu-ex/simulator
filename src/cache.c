#include "cache.h"

s32 cache_get_valid_block(const CACHE* cache, const ADDR addr) {
    register const CACHE_ADDR_HELPER helper = { .raw = addr };
    register const s32 idx = helper.d.set_idx * ASSOCIATIVITY;
    register const u32 tag = helper.d.tag;
    #if (ASSOCIATIVITY == 1)
    if (cache->blocks[idx]->valid && (cache->blocks[idx]->tag == tag)) return idx;
    #elif (ASSOCIATIVITY == 2)
    if (cache->blocks[idx]->valid && (cache->blocks[idx]->tag == tag)) return idx;
    if (cache->blocks[idx + 1]->valid && (cache->blocks[idx + 1]->tag == tag)) return idx + 1;
    #elif (ASSOCIATIVITY == 4)
    if (cache->blocks[idx]->valid && (cache->blocks[idx]->tag == tag)) return idx;
    if (cache->blocks[idx + 1]->valid && (cache->blocks[idx + 1]->tag == tag)) return idx + 1;
    if (cache->blocks[idx + 2]->valid && (cache->blocks[idx + 2]->tag == tag)) return idx + 2;
    if (cache->blocks[idx + 3]->valid && (cache->blocks[idx + 3]->tag == tag)) return idx + 3;
    #else
    for (register int i = 0; i < ASSOCIATIVITY; ++i) {
        if (!cache->blocks[idx + i]->valid) continue;
        if (cache->blocks[idx + i]->tag ^ tag) continue;
        return idx + i;
    }
    #endif
    return -1;
}

s32 cache_get_empty_block(const CACHE* cache, const ADDR addr) {
    register const s32 idx = (CACHE_ADDR_HELPER){ .raw = addr }.d.set_idx * ASSOCIATIVITY;
    #if (ASSOCIATIVITY == 1)
    if (!cache->blocks[idx]->valid) return idx;
    #elif (ASSOCIATIVITY == 2)
    if (!cache->blocks[idx]->valid) return idx;
    if (!cache->blocks[idx + 1]->valid) return idx + 1;
    #elif (ASSOCIATIVITY == 4)
    if (!cache->blocks[idx]->valid) return idx;
    if (!cache->blocks[idx + 1]->valid) return idx + 1;
    if (!cache->blocks[idx + 2]->valid) return idx + 2;
    if (!cache->blocks[idx + 3]->valid) return idx + 3;
    #else
    for (register int i = 0; i < ASSOCIATIVITY; ++i) {
        if (!cache->blocks[idx + i]->valid)
            return idx + i;
    }
    #endif
    return -1;
}

s32 cache_get_replacable_block(const CACHE* cache, const ADDR addr) {
    register const s32 start_idx = (CACHE_ADDR_HELPER){ .raw = addr }.d.set_idx * ASSOCIATIVITY;
    register s32 block_idx;
    #if defined(CACHE_FIFO) // fifo
    register u64 min = 0xFFFFFFFFFFFFFFFF;
    for (register int i = 0; i < ASSOCIATIVITY; ++i) {
        if (cache->blocks[start_idx + i]->start_point < min) {
            min = cache->blocks[start_idx + i]->start_point;
            block_idx = start_idx + i;
        }
    }
    #elif defined(CACHE_LRU) // lru
    #if (ASSOCIATIVITY == 1)
    block_idx = start_idx;
    #elif (ASSOCIATIVITY == 2)
    block_idx = start_idx + ((cache->blocks[start_idx]->last_referenced < cache->blocks[start_idx + 1]->last_referenced) ? 0 : 1);
    #else
    register u64 min = 0xFFFFFFFFFFFFFFFF;
    for (register int i = 0; i < ASSOCIATIVITY; ++i) {
        if (cache->blocks[start_idx + i]->last_referenced < min) {
            min = cache->blocks[start_idx + i]->last_referenced;
            block_idx = start_idx + i;
        }
    }
    #endif
    #elif defined(CACHE_RR) // round robin
    register u64 max = 0;
    for (register int i = 0; i < ASSOCIATIVITY; ++i) {
        if (cache->blocks[start_idx + i]->last_referenced > max) {
            max = cache->blocks[start_idx + i]->last_referenced;
            block_idx = start_idx + i;
        }
    }
    block_idx = start_idx + (block_idx - start_idx + 1) % ASSOCIATIVITY;
    #else // 1st entry
    block_idx = start_idx;
    #endif
    return block_idx;
}

u32 cache_read_word(CACHE* const cache, const ADDR addr, WORD* const val) {
    ++cache->reference_counter;
    ++cache->read_counter;
    register const s32 block_idx = cache_get_valid_block(cache, addr);
    if (block_idx < 0) {
        // miss
        ++cache->miss_counter;
        return 1;
    } else {
        // hit
        ++cache->hit_counter;
        *val = cache->blocks[block_idx]->data[(CACHE_ADDR_HELPER){ .raw = addr }.d.offset];
        cache->blocks[block_idx]->last_referenced = cache->reference_counter;
        return 0;
    }
}

u32 cache_write_word(CACHE* const cache, const ADDR addr, const WORD val) {
    ++cache->reference_counter;
    ++cache->write_counter;
    register const s32 block_idx = cache_get_valid_block(cache, addr);
    if (block_idx < 0) {
        // miss
        ++cache->miss_counter;
        return 1;
    } else {
        // hit
        ++cache->hit_counter;
        cache->blocks[block_idx]->data[(CACHE_ADDR_HELPER){ .raw = addr }.d.offset] = val;
        cache->blocks[block_idx]->modified = 1;
        cache->blocks[block_idx]->last_referenced = cache->reference_counter;
        return 0;
    }
}

void cache_load_block(CACHE* const cache, const ADDR addr, const MEM* mem) {
    register s32 block_idx = cache_get_empty_block(cache, addr);
    register CACHE_ADDR_HELPER helper;
    // write back
    if (block_idx < 0) {
        // all blocks occupied
        block_idx = cache_get_replacable_block(cache, addr);
        if (cache->blocks[block_idx]->modified) {
            helper.d.tag = cache->blocks[block_idx]->tag;
            helper.d.set_idx = cache->blocks[block_idx]->set_idx;
            for (register int offset = 0; offset < BLOCK_SIZE; ++offset) {
                helper.d.offset = offset;
                mem->write_word((MEM* const)mem, helper.raw, cache->blocks[block_idx]->data[offset]);
            }
        }
    }
    // load block
    helper.raw = addr;
    cache->blocks[block_idx]->valid = 1;
    cache->blocks[block_idx]->modified = 0;
    cache->blocks[block_idx]->start_point = cache->reference_counter;
    cache->blocks[block_idx]->last_referenced = cache->read_counter;
    cache->blocks[block_idx]->tag = helper.d.tag;
    cache->blocks[block_idx]->set_idx = helper.d.set_idx;
    for (register int offset = 0; offset < BLOCK_SIZE; ++offset) {
        helper.d.offset = offset;
        cache->blocks[block_idx]->data[offset] = mem->read_word(mem, helper.raw);
    }
}

// get data without incrementing counters
u32 cache_sneak(CACHE* cache, ADDR addr, WORD* val) {
    s32 block_idx = cache_get_valid_block(cache, addr);
    if (block_idx < 0) {
        // miss
        return 1;
    } else {
        // hit
        CACHE_ADDR_HELPER helper = { .raw = addr };
        *val = cache->blocks[block_idx]->data[helper.d.offset];
        return 0;
    }
}

void cache_reset(CACHE* cache) {
    cache->hit_counter = 0;
    cache->miss_counter = 0;
    cache->read_counter = 0;
    cache->write_counter = 0;
    cache->reference_counter = 0;
    for (int idx = 0; idx < BLOCK_NUM; ++idx) {
        if (cache->blocks[idx] != NULL)
            free(cache->blocks[idx]);
        cache->blocks[idx] = (CACHE_BLOCK*)malloc(sizeof(CACHE_BLOCK));
        memset(cache->blocks[idx], 0, sizeof(CACHE_BLOCK));
    }
}

void init_cache(CACHE* cache) {
    // check BLOCK_SIZE
    if (BLOCK_SIZE & 0x3) {
        printf("cache: block size should be the multiple of 4(words).\n");
        exit(-1);
    } else if (ASSOCIATIVITY == 0) {
        printf("cache: associativity (AKA way) should be at least 1.\n");
        exit(-1);
    }
    // setup parameters
    cache_reset(cache);
    // assign interfaces
    cache->read_word = cache_read_word;
    cache->write_word = cache_write_word;
    cache->sneak = cache_sneak;
    cache->load_block = cache_load_block;
    cache->reset = cache_reset;
}
