#include "cache.h"

// #define is_valid_setting (((TAG_LEN / 8 + BLOCK_SIZE) * BLOCK_NUM) <= CACHE_SIZE ? 1 : 0)

s32 cache_get_valid_block(CACHE* cache, ADDR addr) {
    CACHE_ADDR_HELPER helper = { .raw = addr };
    register s32 idx = helper.d.set_idx * ASSOCIATIVITY;
    register u32 tag = helper.d.tag;
    for (register int i = 0; i < ASSOCIATIVITY; i++, idx++) {
        if (!cache->blocks[idx]->valid) continue;
        if (cache->blocks[idx]->tag ^ tag) continue;
        return idx;
    }
    return -1;
}

s32 cache_get_empty_block(CACHE* cache, ADDR addr) {
    CACHE_ADDR_HELPER helper = { .raw = addr };
    register s32 idx = helper.d.set_idx * ASSOCIATIVITY;
    for (register int i = 0; i < ASSOCIATIVITY; i++, idx++) {
        if (!cache->blocks[idx]->valid)
            return idx;
    }
    return -1;
}

s32 cache_get_replacable_block(CACHE* cache, ADDR addr) {
    CACHE_ADDR_HELPER helper = { .raw = addr };
    register s32 start_idx = helper.d.set_idx * ASSOCIATIVITY, block_idx;
    #if defined(CACHE_FIFO) // fifo
    register u32 min = 0xFFFFFFFF;
    for (register int i = 0; i < ASSOCIATIVITY; i++) {
        if (cache->blocks[start_idx + i]->start_point < min) {
            min = cache->blocks[start_idx + i]->start_point;
            block_idx = start_idx + i;
        }
    }
    #elif defined(CACHE_LRU) // lru
    register u32 min = 0xFFFFFFFF;
    for (register int i = 0; i < ASSOCIATIVITY; i++) {
        if (cache->blocks[start_idx + i]->last_referenced < min) {
            min = cache->blocks[start_idx + i]->last_referenced;
            block_idx = start_idx + i;
        }
    }
    #elif defined(CACHE_RR) // round robin
    register u32 max = 0;
    for (register int i = 0; i < ASSOCIATIVITY; i++) {
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

u8 cache_read_byte(CACHE* cache, ADDR addr, BYTE* val) {
    cache->reference_counter++;
    cache->read_counter++;
    register s32 block_idx = cache_get_valid_block(cache, addr);
    if (block_idx < 0) {
        // miss
        cache->miss_counter++;
        return 0;
    } else {
        // hit
        cache->hit_counter++;
        CACHE_ADDR_HELPER helper = { .raw = addr };
        *val = cache->blocks[block_idx]->data[helper.d.offset];
        cache->blocks[block_idx]->last_referenced = cache->reference_counter;
        return 1;
    }
}

u8 cache_write_byte(CACHE* cache, ADDR addr, BYTE val) {
    cache->reference_counter++;
    cache->write_counter++;
    register s32 block_idx = cache_get_valid_block(cache, addr);
    if (block_idx < 0) {
        // miss
        cache->miss_counter++;
        return 0;
    } else {
        // hit
        cache->hit_counter++;
        CACHE_ADDR_HELPER helper = { .raw = addr };
        cache->blocks[block_idx]->data[helper.d.offset] = val;
        cache->blocks[block_idx]->modified = 1;
        cache->blocks[block_idx]->last_referenced = cache->reference_counter;
        return 1;
    }
}

void cache_load_block(CACHE* cache, ADDR addr, MEM* mem) {
    register s32 block_idx = cache_get_empty_block(cache, addr);
    CACHE_ADDR_HELPER helper;
    // write back
    if (block_idx < 0) {
        // all blocks occupied
        block_idx = cache_get_replacable_block(cache, addr);
        if (cache->blocks[block_idx]->modified) {
            helper.d.tag = cache->blocks[block_idx]->tag;
            helper.d.set_idx = cache->blocks[block_idx]->set_idx;
            for (register int offset = 0; offset < BLOCK_SIZE; offset++) {
                helper.d.offset = offset;
                mem->write_byte(mem, helper.raw, cache->blocks[block_idx]->data[offset]);
                cache->blocks[block_idx]->data[offset] = 0;
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
    for (register int offset = 0; offset < BLOCK_SIZE; offset++) {
        helper.d.offset = offset;
        cache->blocks[block_idx]->data[offset] = mem->read_byte(mem, helper.raw);
    }
}

// get data without incrementing counters
u8 cache_sneak(CACHE* cache, ADDR addr, BYTE* val) {
    s32 block_idx = cache_get_valid_block(cache, addr);
    if (block_idx < 0) {
        // miss
        return 0;
    } else {
        // hit
        CACHE_ADDR_HELPER helper = { .raw = addr };
        *val = cache->blocks[block_idx]->data[helper.d.offset];
        return 1;
    }
}

void cache_reset(CACHE* cache) {
    cache->hit_counter = 0;
    cache->miss_counter = 0;
    cache->read_counter = 0;
    cache->write_counter = 0;
    cache->reference_counter = 0;
    for (int idx = 0; idx < BLOCK_NUM; idx++) {
        if (cache->blocks[idx] != NULL)
            free(cache->blocks[idx]);
        cache->blocks[idx] = (CACHE_BLOCK*)malloc(sizeof(CACHE_BLOCK));
    }
}

void init_cache(CACHE* cache) {
    // if (!is_valid_setting) {
    //     printf("cache setting is not valid.\n");
    //     exit(-1);
    // }
    cache_reset(cache);
    // assign interfaces
    cache->read_byte = cache_read_byte;
    cache->write_byte = cache_write_byte;
    cache->sneak = cache_sneak;
    cache->load_block = cache_load_block;
    cache->reset = cache_reset;
}
