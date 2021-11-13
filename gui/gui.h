#pragma once
#include <ncurses.h>
#include "../src/types.h"
#include "../src/core.h"
#include "../src/instr.h"

#define TITLE_COLOR     1
#define SUBTITLE_COLOR  2
#define WARNING_COLOR   3
#define HIGHLIGHT_COLOR 4
#define STANDOUT_COLOR  5

#define REG_SET_DEF 0
#define REG_SET_A   1
#define REG_SET_S   2
#define REG_SET_T   3
#define REG_SET_FA  4
#define REG_SET_FS  5
#define REG_SET_FT  6

#define MEM_INSTR 1
#define MEM_DATA  0

static int reg_set[7][17] = {
    {16, zero, ra, sp, gp, tp, t0, t1, t2, fp, s1, a0, a1, a2, a3, a4, a5},
    {8, a0, a1, a2, a3, a4, a5, a6, a7},
    {12, s0, s1, s2, s3, s4, s5, s6, s7, s8, s9, s10, s11},
    {7, t0, t1, t2, t3, t4, t5, t6},
    {8, 32 + fa0, 32 + fa1, 32 + fa2, 32 + fa3, 32 + fa4, 32 + fa5, 32 + fa6, 32 + fa7},
    {12, 32 + fs0, 32 + fs1, 32 + fs2, 32 + fs3, 32 + fs4, 32 + fs5, 32 + fs6, 32 + fs7, 32 + fs8, 32 + fs9, 32 + fs10, 32 + fs11},
    {12, 32 + ft0, 32 + ft1, 32 + ft2, 32 + ft3, 32 + ft4, 32 + ft5, 32 + ft6, 32 + ft7, 32 + ft8, 32 + ft9, 32 + ft10, 32 + ft11}
};

typedef struct gui {
    // display control
    u8 reg_set;
    u8 reg_focus[64];
    u8 mem_type;
    ADDR mem_start;
    ADDR mem_focus;
    // interfaces
    STATE (*update)(CORE* core);
    void (*deinit)(void);
} GUI;

void init_gui(GUI* win);
