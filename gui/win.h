#pragma once
#include <ncurses.h>
#include "../src/types.h"
#include "../src/core.h"

#define REG_SET_DEF 0
#define REG_SET_A   1
#define REG_SET_S   2
#define REG_SET_T   3

static int reg_set[4][17] = {
    {16, zero, ra, sp, gp, tp, t0, t1, t2, fp, s1, a0, a1, a2, a3, a4, a5},
    {8, a0, a1, a2, a3, a4, a5, a6, a7},
    {12, s0, s1, s2, s3, s4, s5, s6, s7, s8, s9, s10, s11},
    {7, t0, t1, t2, t3, t4, t5, t6}
};

typedef struct win {
    WINDOW* pc_win;
    WINDOW* reg_win;
    WINDOW* mem_win;
    WINDOW* com_win;

    // display control
    char reg_set;
    char reg_focus[32];
    ADDR mem_start;
    ADDR mem_focus;

    STATE (*update)(CORE* core);
    void (*deinit)(void);
} WIN;

void init_win(WIN* win);
