#pragma once
#include "types.h"

#define CACHE_LEN 0x400

typedef struct mmu {
    BYTE* cache;

    BYTE (*read_byte)(ADDR);
    HALF (*read_half)(ADDR);
    WORD (*read_word)(ADDR);
    void (*write_byte)(ADDR, BYTE);
    void (*write_half)(ADDR, HALF);
    void (*write_word)(ADDR, WORD);
} MMU;

void init_mmu(MMU* mmu);
