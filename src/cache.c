#include "cache.h"

static CACHE* cache_base;

// #define is_valid_setting (((TAG_LEN / 8 + BLOCK_SIZE) * BLOCK_NUM) <= CACHE_SIZE ? 1 : 0)

int cache_get_valid_block(ADDR addr) {
    CACHE_ADDR_HELPER helper = { .raw = addr };
    int idx = helper.d.set_idx * ASSOCIATIVITY;
    for (int i = 0; i < ASSOCIATIVITY; i++, idx++) {
        if (cache_base->blocks[idx].valid && cache_base->blocks[idx].tag == helper.d.tag)
            return idx;
    }
    return -1;
}

int cache_get_empty_block(ADDR addr) {
    CACHE_ADDR_HELPER helper = { .raw = addr };
    int idx = helper.d.set_idx * ASSOCIATIVITY;
    for (int i = 0; i < ASSOCIATIVITY; i++, idx++) {
        if (!cache_base->blocks[idx].valid)
            return idx;
    }
    return -1;
}

int cache_get_replacable_block(ADDR addr) {
    CACHE_ADDR_HELPER helper = { .raw = addr };
    int start_idx = helper.d.set_idx * ASSOCIATIVITY, block_idx;
    #if defined(CACHE_FIFO) // fifo
    u32 min = 0xFFFFFFFF;
    for (int i = 0; i < ASSOCIATIVITY; i++) {
        if (cache_base->blocks[start_idx + i].start_point < min) {
            min = cache_base->blocks[start_idx + i].start_point;
            block_idx = start_idx + i;
        }
    }
    #elif defined(CACHE_LRU) // lru
    u32 min = 0xFFFFFFFF;
    for (int i = 0; i < ASSOCIATIVITY; i++) {
        if (cache_base->blocks[start_idx + i].last_referenced < min) {
            min = cache_base->blocks[start_idx + i].last_referenced;
            block_idx = start_idx + i;
        }
    }
    #elif defined(CACHE_RR) // round robin
    u32 max = 0;
    for (int i = 0; i < ASSOCIATIVITY; i++) {
        if (cache_base->blocks[start_idx + i].last_referenced > max) {
            max = cache_base->blocks[start_idx + i].last_referenced;
            block_idx = start_idx + i;
        }
    }
    block_idx = start_idx + (block_idx - start_idx + 1) % ASSOCIATIVITY;
    #else // 1st entry
    block_idx = start_idx;
    #endif
    fprintf(stderr, "%u\n", block_idx);
    return block_idx;
}

u8 cache_read_byte(ADDR addr, BYTE* val) {
    cache_base->reference_counter++;
    cache_base->read_counter++;
    int block_idx = cache_get_valid_block(addr);
    if (block_idx < 0) {
        // miss
        cache_base->miss_counter++;
        return 0;
    } else {
        // hit
        cache_base->hit_counter++;
        CACHE_ADDR_HELPER helper = { .raw = addr };
        *val = cache_base->blocks[block_idx].data[helper.d.offset];
        cache_base->blocks[block_idx].last_referenced = cache_base->reference_counter;
        return 1;
    }
}

u8 cache_write_byte(ADDR addr, BYTE val) {
    cache_base->reference_counter++;
    cache_base->write_counter++;
    int block_idx = cache_get_valid_block(addr);
    if (block_idx < 0) {
        // miss
        cache_base->miss_counter++;
        return 0;
    } else {
        // hit
        cache_base->hit_counter++;
        CACHE_ADDR_HELPER helper = { .raw = addr };
        cache_base->blocks[block_idx].data[helper.d.offset] = val;
        cache_base->blocks[block_idx].modified = 1;
        cache_base->blocks[block_idx].last_referenced = cache_base->reference_counter;
        return 1;
    }
}

void cache_load_block(ADDR addr, BYTE (*reader)(ADDR), void (*writer)(ADDR, BYTE)) {
    int block_idx = cache_get_empty_block(addr);
    CACHE_ADDR_HELPER helper;
    // write back
    if (block_idx < 0) {
        // all blocks occupied
        block_idx = cache_get_replacable_block(addr);
        if (cache_base->blocks[block_idx].modified) {
            helper.d.tag = cache_base->blocks[block_idx].tag;
            helper.d.set_idx = cache_base->blocks[block_idx].set_idx;
            for (int offset = 0; offset < BLOCK_SIZE; offset++) {
                helper.d.offset = offset;
                writer(helper.raw, cache_base->blocks[block_idx].data[offset]);
                cache_base->blocks[block_idx].data[offset] = 0;
            }
        }
    }
    // load block
    helper.raw = addr;
    cache_base->blocks[block_idx].valid = 1;
    cache_base->blocks[block_idx].modified = 0;
    cache_base->blocks[block_idx].start_point = cache_base->reference_counter;
    cache_base->blocks[block_idx].last_referenced = cache_base->read_counter;
    cache_base->blocks[block_idx].tag = helper.d.tag;
    cache_base->blocks[block_idx].set_idx = helper.d.set_idx;
    for (int offset = 0; offset < BLOCK_SIZE; offset++) {
        helper.d.offset = offset;
        cache_base->blocks[block_idx].data[offset] = reader(helper.raw);
    }
}

// get data without incrementing counters
u8 cache_sneak(ADDR addr, BYTE* val) {
    int block_idx = cache_get_valid_block(addr);
    if (block_idx < 0) {
        // miss
        return 0;
    } else {
        // hit
        CACHE_ADDR_HELPER helper = { .raw = addr };
        *val = cache_base->blocks[block_idx].data[helper.d.offset];
        return 1;
    }
}

void cache_reset() {
    cache_base->hit_counter = 0;
    cache_base->miss_counter = 0;
    cache_base->read_counter = 0;
    cache_base->write_counter = 0;
    cache_base->reference_counter = 0;
    for (int idx = 0; idx < BLOCK_NUM; idx++)
        cache_base->blocks[idx].valid = 0;
}

void init_cache(CACHE* cache) {
    // if (!is_valid_setting) {
    //     printf("cache setting is not valid.\n");
    //     exit(-1);
    // }
    cache_base = cache;
    cache_reset();
    // assign interfaces
    cache->read_byte = cache_read_byte;
    cache->write_byte = cache_write_byte;
    cache->sneak = cache_sneak;
    cache->load_block = cache_load_block;
    cache->reset = cache_reset;
}
