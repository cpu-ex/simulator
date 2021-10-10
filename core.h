#pragma once
#include "types.h"
#include "mem.h"

enum reg {
    zero = 0,
    ra,
    sp,
    gp,
    tp,
    t0, t1, t2,
    s0, s1,
    a0, a1, a2, a3, a4, a5, a6, a7,
    s2, s3, s4, s5, s6, s7, s8, s9, s10, s11,
    t3, t4, t5, t6,
    fp = 8
};

static char* reg_name[32] = {
    "z",
    "ra",
    "sp",
    "gp",
    "tp",
    "t0", "t1", "t2",
    "s0", "s1",
    "a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7",
    "s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9", "sA", "sB",
    "t3", "t4", "t5", "t6"
};

typedef struct core {
    REG pc;
    REG regs[32];
    
    u_int64_t instr_counter;

    MMU* mmu;

    WORD (*load)(ADDR, int ,int);
    void (*store)(ADDR, WORD, int);
    void (*if_dc_ex)(void);
} CORE;

void init_core(CORE* core, ADDR pc);
