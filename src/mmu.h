#pragma once
#include "global.h"
#include "mem.h"
#include "cache.h"

typedef struct mmu {
    // attributes
    u32 instr_len;
    WORD* instr_mem;
    CACHE* data_cache;
    MEM* data_mem;
    // interfaces
    void (*allocate_instr)(struct mmu*, u64);
    WORD (*read_instr)(struct mmu*, ADDR);
    void (*write_instr)(struct mmu*, ADDR, WORD);
    BYTE (*read_data)(struct mmu*, void*, ADDR);
    void (*write_data)(struct mmu*, void*, ADDR, BYTE);
    BYTE (*sneak)(struct mmu*, ADDR, u8);
    void (*reset)(struct mmu*, ADDR);
} MMU;

void init_mmu(MMU* mmu);
