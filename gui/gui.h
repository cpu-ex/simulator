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

#define MEM_INSTR 1
#define MEM_DATA  0

typedef struct gui {
    // display control
    u8 focused_win;
    int reg_start;
    u8 reg_focus[64];
    ADDR mem_start;
    u8 mem_type;
    // interfaces
    STATE (*update)(struct gui*, CORE*);
    void (*deinit)(struct gui*);
} GUI;

void init_gui(GUI* win);
