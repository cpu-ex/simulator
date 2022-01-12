#pragma once
#include "global.h"

#define MAX_ADDR 0x04000000

// address
// 31 ~ 26 (6): disused
// 25 ~ 18 (8): 1st index
// 17 ~ 10 (8): 2nd index
// 9  ~ 2  (8): page

typedef struct mem {
    WORD** data[0x100];
    // interfaces
    WORD (*read_word)(struct mem*, ADDR);
    void (*write_word)(struct mem*, ADDR, WORD);
    void (*reset_stack)(struct mem*, ADDR);
} MEM;

void init_mem(MEM* mem);
