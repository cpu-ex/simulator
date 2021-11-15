#pragma once
#include "types.h"

#define STACK_POINTER  0x03FFFFF0

typedef struct mmu {
    // attributes
    BYTE* instr_mem;
    BYTE* data_mem;
    BYTE* stack;

    u32 instr_len;
    u32 data_len;
    u32 stack_len;
    // interfaces
    void (*allocate_instr)(u64);
    void (*allocate_data)(u64);
    void (*allocate_stack)(u64);

    BYTE (*read_instr)(ADDR);
    BYTE (*read_data)(ADDR);
    void (*write_instr)(ADDR, BYTE);
    void (*write_data)(ADDR, BYTE);
    void (*reset)(void);
} MMU;

void init_mmu(MMU* mmu);
