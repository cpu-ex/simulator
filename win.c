#include <stdio.h>
#include "win.h"

static WIN* win_base;

void wait() { getch(); }
void deinit() { endwin(); }

void init_win(WIN* win) {
    win_base = win;

    // resize the terminal to 80 * 24
    printf("\e[8;24;80t");
    // initialize ncurses mode, and end it in deinit()
    initscr();
    // create outer windows (containers that never change)
    WINDOW* pc_outer = newwin(3, 80, 0, 0);
    WINDOW* reg_outer = newwin(18, 14, 3, 0);
    WINDOW* mem_outer = newwin(18, 66, 3, 14);
    WINDOW* com_outer = newwin(3, 80, 21, 0);
    refresh();
    // box the containers and name the titles
    box(pc_outer, 0, 0); mvwprintw(pc_outer, 0, 2, "PC"); wrefresh(pc_outer);
    box(reg_outer, 0, 0); mvwprintw(reg_outer, 0, 2, "Registers"); wrefresh(reg_outer);
    box(mem_outer, 0, 0); mvwprintw(mem_outer, 0, 2, "Memory"); wrefresh(mem_outer);
    box(com_outer, 0, 0); mvwprintw(com_outer, 0, 2, "Input"); wrefresh(com_outer);

    win->wait = wait;
    win->deinit = deinit;
}
