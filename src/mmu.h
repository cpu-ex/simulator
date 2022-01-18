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
    u8 is_nocache;
    // interfaces
    void (*allocate_instr)(struct mmu*, u64);
    const WORD (*read_instr)(const struct mmu*, const ADDR);
    const WORD (*read_data)(const struct mmu*, void* const, const ADDR);
    void (*write_instr)(const struct mmu*, const ADDR, const WORD);
    void (*write_data)(const struct mmu*, void* const, const ADDR, const WORD);
    BYTE (*sneak)(struct mmu*, ADDR, u8);
    void (*reset)(struct mmu*, ADDR);
} MMU;

void init_mmu(MMU* mmu, u8 is_nocache);
