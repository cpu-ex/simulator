#pragma once
#include "global.h"
#include "mem.h"

#define LOG_1(n) (((n) < 1 << 1) ? 0 : 1)
#define LOG_2(n) (((n) < 1 << 2) ? LOG_1(n) : (2 + LOG_1((n) >> 2)))
#define LOG_4(n) (((n) < 1 << 4) ? LOG_2(n) : (4 + LOG_2((n) >> 4)))
#define LOG_8(n) (((n) < 1 << 8) ? LOG_4(n) : (8 + LOG_4((n) >> 8)))
#define LOG(n)   (((n) < 1 << 16) ? LOG_8(n) : (16 + LOG_8((n) >> 16)))

// customizable variables
#define BLOCK_SIZE    64  // in bytes
#define ASSOCIATIVITY 1    // aka way
#define SET_NUM       2048 // aka depth
// #define CACHE_FIFO
#define CACHE_LRU
// #define CACHE_RR // round robin

// constants
#define BLOCK_NUM   (SET_NUM * ASSOCIATIVITY)
#define OFFSET_LEN  LOG(BLOCK_SIZE)
#define SET_IDX_LEN LOG(SET_NUM)
#define TAG_LEN     (32 - SET_IDX_LEN - OFFSET_LEN)

typedef union cache_addr_helper {
    u32 raw;

    struct addr_decoder {
        u32 offset : OFFSET_LEN;
        u32 set_idx : SET_IDX_LEN;
        u32 tag : TAG_LEN;
    } __attribute__((packed)) d;
} CACHE_ADDR_HELPER;

typedef struct cache_block {
    u8 valid;
    u8 modified;
    u32 start_point; // the first time the block was settled
    u32 last_referenced;
    u32 tag;
    u32 set_idx;
    u8 data[BLOCK_SIZE];
} CACHE_BLOCK;

typedef struct cache {
    // attributes
    CACHE_BLOCK* blocks[BLOCK_NUM];
    u32 hit_counter;
    u32 miss_counter;
    u32 read_counter;
    u32 write_counter;
    u32 reference_counter;
    // interfaces
    u8 (*read_byte)(struct cache*, ADDR, BYTE*);
    u8 (*write_byte)(struct cache*, ADDR, BYTE);
    u8 (*sneak)(struct cache*, ADDR, BYTE*);
    void (*load_block)(struct cache*, ADDR, MEM*);
    void (*reset)(struct cache*);
} CACHE;

void init_cache(CACHE* cache);
