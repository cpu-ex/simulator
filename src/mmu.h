#pragma once
#include "types.h"
#include "mem.h"
#include "cache.h"

typedef struct mmu {
    // attributes
    u32 instr_len;
    BYTE* instr_mem;
    CACHE* data_cache;
    MEM* data_mem;
    // interfaces
    void (*allocate_instr)(u64);
    BYTE (*read_instr)(ADDR);
    void (*write_instr)(ADDR, BYTE);
    BYTE (*read_data)(ADDR);
    void (*write_data)(ADDR, BYTE);
    BYTE (*sneak)(ADDR, u8);
    void (*reset)(ADDR);
} MMU;

void init_mmu(MMU* mmu);
