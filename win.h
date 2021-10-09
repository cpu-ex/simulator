#pragma once
#include <ncurses.h>
#include "types.h"
#include "core.h"

typedef struct win {
    WINDOW* pc_win;
    WINDOW* reg_win;
    WINDOW* mem_win;
    WINDOW* com_win;

    void (*wait)(void);
    void (*deinit)(void);
} WIN;

void init_win(WIN* win);
