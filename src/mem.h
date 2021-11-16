#pragma once
#include "types.h"

// address
// 31 ~ 26 (6 ): disused
// 25 ~ 18 (8 ): 1st index
// 17 ~ 8  (10): 2nd index
// 7  ~ 1  (8 ): page

typedef struct mem {
    BYTE** data[0x100];
    // interfaces
    BYTE (*read_byte)(ADDR);
    void (*write_byte)(ADDR, BYTE);
    void (*reset_stack)(ADDR);
} MEM;

void init_mem(MEM* mem);
