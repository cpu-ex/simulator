#pragma once
#include <ncurses.h>
#include "../src/types.h"
#include "../src/core.h"
#include "../src/instr.h"

// customizable variables
// timeout time in msec
// negative number for blocking
// 0 for non-blocking (do not wait for input at all)
// positive number for time-limited blocking
// (wait for 1s when setting to 1000, if input is none then skip it)
#define TIMEOUT_TIME   -1

#define TITLE_COLOR     1
#define SUBTITLE_COLOR  2
#define WARNING_COLOR   3
#define HIGHLIGHT_COLOR 4
#define STANDOUT_COLOR  5

#define REG_WIN 0
#define MEM_WIN 1
#define COM_WIN 2

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
