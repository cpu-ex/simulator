#pragma once
#include "types.h"

#define STACK_POINTER 0xBFFFFFF0

typedef struct mmu {
    BYTE* instr_cache;
    BYTE* data_cache;
    BYTE* data_mem;
    BYTE* stack;

    u32 instr_len;
    u32 data_len;
    u32 stack_len;

    void (*allocate_instr)(u64);
    void (*allocate_data)(u64);
    void (*allocate_stack)(u64);

    BYTE (*read_byte)(ADDR);
    HALF (*read_half)(ADDR);
    WORD (*read_word)(ADDR);
    void (*write_byte)(ADDR, BYTE);
    void (*write_half)(ADDR, HALF);
    void (*write_word)(ADDR, WORD);

    void (*reset)(void);
} MMU;

void init_mmu(MMU* mmu);
