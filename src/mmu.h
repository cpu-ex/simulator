#pragma once
#include "types.h"
#include "mem.h"

typedef struct mmu {
    // attributes
    BYTE* instr_mem;
    u32 instr_len;
    MEM* data_mem;
    // interfaces
    void (*allocate_instr)(u64);
    BYTE (*read_instr)(ADDR);
    void (*write_instr)(ADDR, BYTE);
    BYTE (*read_data)(ADDR);
    void (*write_data)(ADDR, BYTE);
    void (*reset)(ADDR);
} MMU;

void init_mmu(MMU* mmu);
