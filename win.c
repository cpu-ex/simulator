#include <stdio.h>
#include <string.h>
#include "win.h"

#define TITLE_COLOR 1
#define SUBTITLE_COLOR 2
#define STANDOUT_COLOR 3

static WIN* win_base;

STATE wait() {
    char input[72];
    wgetstr(win_base->com_win, input);
    if (!strcmp(input, "quit"))
        return QUIT;
    else
        return NEXT;
}

void update_pc(CORE* core) {
    WINDOW *win = win_base->pc_win;
    wclear(win);
    ADDR pc = core->pc;
    WORD op = core->load(pc, 2, 0);
    wprintw(win, " 0x%08X : %08X", pc, op);
}

void update_reg(CORE* core) {
    WINDOW* win = win_base->reg_win;
    wclear(win);
    for (int reg = zero; reg < a6; reg++) {
        wattron(win, COLOR_PAIR(SUBTITLE_COLOR));
        if (reg == 8)
            wprintw(win, "fp");
        else
            wprintw(win, "%2s", reg_name[reg]);
        wattroff(win, COLOR_PAIR(SUBTITLE_COLOR));
        wprintw(win, "%9X\n", core->regs[reg]);
    }
}

void update_mem(CORE* core) {
    WINDOW* win = win_base->mem_win;
    wclear(win);
    ADDR addr = 0;
    BYTE val = 0;
    for (ADDR offset = 0; offset < 0x100; offset++) {
        wattron(win, COLOR_PAIR(SUBTITLE_COLOR));
        if (offset % 0x10 == 0)
            wprintw(win, "0x%08X     ", addr + offset);
        wattroff(win, COLOR_PAIR(SUBTITLE_COLOR));
        val = core->load(addr + offset, 0, 0);
        wprintw(win, " %02X", val);
        if (offset % 0x10 == 0xF)
            wprintw(win, "\n");
    }
}

void update_com(CORE* core) {
    WINDOW *win = win_base->com_win;
    wclear(win);
    wmove(win, 0, 0);
}

STATE update(CORE* core) {
    update_pc(core);
    update_reg(core);
    update_mem(core);
    update_com(core);
    // refresh routine
    refresh();
    wrefresh(win_base->pc_win);
    wrefresh(win_base->reg_win);
    wrefresh(win_base->mem_win);
    wrefresh(win_base->com_win);
    // wait for a new command
    return wait();
}

void show_splash_info() {
    // some simple instructions of using simulator
    WINDOW* win = win_base->com_win;
    char info[] = "-> press any key but 'h' to skip <-";
    wattron(win, COLOR_PAIR(STANDOUT_COLOR));
    mvwprintw(win, 0, (78 - strlen(info)) / 2, info);
    wattroff(win, COLOR_PAIR(STANDOUT_COLOR));
    refresh();
    wrefresh(win);
    if (wgetch(win) == 'h') {
        // TODO: display a detailed instruction screen
    }
}

void deinit() {
    // at the end of program
    // ncurses mode should be exited properly
    endwin();
}

void init_win(WIN* win) {
    win_base = win;

    // resize the terminal to 80 * 24
    printf("\e[8;24;80t");
    // initialize ncurses mode, and end it in deinit()
    initscr();
    cbreak();
    // set colors if supported
    if (has_colors()) start_color();
    init_pair(TITLE_COLOR, COLOR_BLUE, COLOR_BLACK);
    init_pair(SUBTITLE_COLOR, COLOR_CYAN, COLOR_BLACK);
    init_pair(STANDOUT_COLOR, COLOR_RED, COLOR_BLACK);
    // display a title
    char title[] = " RISC-V simulator ";
    attron(A_STANDOUT); mvprintw(1, 1, title); attroff(A_STANDOUT);
    // create outer windows (containers that never change)
    WINDOW* pc_outer = newwin(3, 78 - strlen(title), 0, 2 + strlen(title));
    WINDOW* reg_outer = newwin(18, 14, 3, 0);
    WINDOW* mem_outer = newwin(18, 66, 3, 14);
    WINDOW* com_outer = newwin(3, 80, 21, 0);
    refresh();
    // box the containers and name the titles
    wattron(pc_outer, COLOR_PAIR(TITLE_COLOR));
    wattron(reg_outer, COLOR_PAIR(TITLE_COLOR));
    wattron(mem_outer, COLOR_PAIR(TITLE_COLOR));
    wattron(com_outer, COLOR_PAIR(TITLE_COLOR));
    box(pc_outer, 0, 0); mvwprintw(pc_outer, 0, 2, " PC "); wrefresh(pc_outer);
    box(reg_outer, 0, 0); mvwprintw(reg_outer, 0, 2, " Register "); wrefresh(reg_outer);
    box(mem_outer, 0, 0); mvwprintw(mem_outer, 0, 2, " Memory "); wrefresh(mem_outer);
    box(com_outer, 0, 0); mvwprintw(com_outer, 0, 2, " Input "); wrefresh(com_outer);
    wattroff(pc_outer, COLOR_PAIR(TITLE_COLOR));
    wattroff(reg_outer, COLOR_PAIR(TITLE_COLOR));
    wattron(mem_outer, COLOR_PAIR(TITLE_COLOR));
    wattron(com_outer, COLOR_PAIR(TITLE_COLOR));
    // create inner windows (contents that actually display unpdates)
    static WINDOW* pc_inner; pc_inner = newwin(1, 76 - strlen(title), 1, 3 + strlen(title));
    static WINDOW* reg_inner; reg_inner = newwin(16, 12, 4, 1);
    static WINDOW* mem_inner; mem_inner = newwin(16, 64, 4, 15);
    static WINDOW* com_inner; com_inner = newwin(1, 77, 22, 2);
    // register inner windows to WIN
    win->pc_win = pc_inner;
    win->reg_win = reg_inner;
    win->mem_win = mem_inner;
    win->com_win = com_inner;

    win->update = update;
    win->show_splash_info = show_splash_info;
    win->deinit = deinit;
}
