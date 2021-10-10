#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "win.h"

#define TITLE_COLOR    1
#define SUBTITLE_COLOR 2
#define STANDOUT_COLOR 3
#define FOCUS_COLOR    4

static WIN* win_base;

void show_help_info() {
    WINDOW* win;

    win = win_base->mem_win;
    wclear(win);
    mvwprintw(win, 1, 3, "step [n]:\n\tmove on for n step, default to 1");
    mvwprintw(win, 3, 3, "reg [d/a/s/t]:\n\tswitch register set, default to zero -> a5");
    mvwprintw(win, 5, 3, "reg [-][reg name]:\n\thighlight certain register,\n\tminus for setting back to normal,\n\tmutiple input supported");
    mvwprintw(win, 9, 3, "mem [address]:\n\tswitch memory range, default to 0x0");
    mvwprintw(win, 11, 3, "help:\n\tshow help window");
    mvwprintw(win, 13, 3, "quit:\n\texit simulator");

    win = win_base->com_win;
    wclear(win);
    char info[] = "-> press any key to skip <-";
    wattron(win, COLOR_PAIR(STANDOUT_COLOR));
    mvwprintw(win, 0, (78 - strlen(info)) / 2, info);
    wattroff(win, COLOR_PAIR(STANDOUT_COLOR));

    refresh();
    wrefresh(win_base->mem_win);
    wgetch(win_base->com_win);
}

void show_splash_info() {
    // some simple instructions of using simulator
    WINDOW* win = win_base->com_win;
    char info[] = "-> press any key but 'h' to skip <-";
    wattron(win, COLOR_PAIR(STANDOUT_COLOR));
    mvwprintw(win, 0, (78 - strlen(info)) / 2, info);
    wattroff(win, COLOR_PAIR(STANDOUT_COLOR));
    refresh();
    if (wgetch(win) == 'h')
        show_help_info();
}

typedef struct command {
    char type;
    int argc;
    int argv[10];
} COMMAND;

int reg2idx(char* reg) {
    if (!strcmp(reg, "fp"))
        return 8;
    int idx;
    for (idx = 0; idx < 32; idx++)
        if (!strcmp(reg, reg_name[idx]))
            break;
    return idx;
}

COMMAND get_command() {
    COMMAND com;
    com.argc = 0;
    // split with space (exactly 1 space)
    char input[73];
    char output[12][12];
    wgetstr(win_base->com_win, input);
    int counter = 0, argc = 0;
    strcat(input, " "); // add an end point
    for (int idx = 0; idx < strlen(input); idx++) {
        if (input[idx] == ' ') {
            output[argc][counter] = '\0';
            argc++;
            counter = 0;
        } else {
            output[argc][counter] = input[idx];
            counter++;
        }
    }
    // pack into COMMAND
    if (!strcmp(output[0], "quit")) {
        com.type = 'q';
    } else if (!strcmp(output[0], "step")) {
        com.type = 's';
        com.argc = 1;
        com.argv[0] = argc > 1 ? atoi(output[1]) : 1;
    } else if (!strcmp(output[0], "reg")) {
        com.type = 'r';
        if (argc > 1) {
            com.argc = argc - 1;
            for (int i = 1; i < argc && i < 11; i++) {
                int flag = 1, idx;
                if (output[i][0] == '-') {
                    // minus for un-focus
                    flag = -1;
                    idx = reg2idx(output[i] + 1);
                } else {
                    idx = reg2idx(output[i]);
                }
                com.argv[i - 1] = idx == 32 ? output[i][0] : (flag * idx);
            }
        } else {
            com.argc = 1;
            com.argv[0] = 'd'; // default reg set
            wprintw(win_base->com_win, "parse as reg, argc = %d", com.argc);
            refresh();
            wgetch(win_base->com_win);
        }
    } else if (!strcmp(output[0], "mem")) {
        com.type = 'm';
        if (argc > 1) {
            com.argc = 1;
            sscanf(output[1], "0x%X", com.argv);
        } else {
            com.argc = 0;
        }
    } else if (!strcmp(output[0], "help")) {
        com.type = 'h';
    } else {
        // parse as step 1
        com.type = 's';
        com.argc = 1;
        com.argv[0] = 1;
    }
    return com;
}

STATE wait4command() {
    COMMAND com = get_command();
    switch (com.type) {
    case 'q':
        return STAT_QUIT;
    case 's':
        return STAT_STEP | (com.argv[0] << 3);
    case 'r':
        for (int i = 0; i < com.argc; i++) {
            int arg = com.argv[i];
            if (arg == 'd') {
                win_base->reg_set = REG_SET_DEF;
            } else if (arg == 'a') {
                win_base->reg_set = REG_SET_A;
            } else if (arg == 's') {
                win_base->reg_set = REG_SET_S;
            } else if (arg == 't') {
                win_base->reg_set = REG_SET_T;
            } else if (32 > arg && arg > -32) {
                if (arg > 0)
                    win_base->reg_focus[arg] = 1;
                else
                    win_base->reg_focus[-arg] = 0;
            }
        }
        return STAT_HALT;
    case 'm':
        win_base->mem_start = com.argc ? com.argv[0] & (~0xFF) : 0x0;
        win_base->mem_focus = com.argc ? com.argv[0] : 0xFFFFFFFF;
        return STAT_HALT;
    case 'h':
        show_help_info();
    default:
        return STAT_HALT;
    }
}

void update_pc(CORE* core) {
    WINDOW *win = win_base->pc_win;
    wclear(win);
    ADDR pc = core->pc;
    WORD op = core->load(pc, 2, 0);
    wprintw(win, "%-5u 0x%08X : %08X", core->instr_counter, pc, op);
}

void update_reg(CORE* core) {
    WINDOW* win = win_base->reg_win;
    wclear(win);

    int count = reg_set[win_base->reg_set][0];
    int* regs = reg_set[win_base->reg_set] + 1;
    for (int idx = 0; idx < count; idx++) {
        wattron(win, COLOR_PAIR(SUBTITLE_COLOR));
        if (regs[idx] == 8 && win_base->reg_set == REG_SET_DEF)
            wprintw(win, "fp ");
        else
            wprintw(win, "%2s ", reg_name[regs[idx]]);
        wattroff(win, COLOR_PAIR(SUBTITLE_COLOR));
        if (win_base->reg_focus[regs[idx]])
            wattron(win, COLOR_PAIR(FOCUS_COLOR));
        wprintw(win, "%8X\n", core->regs[regs[idx]]);
        wattroff(win, COLOR_PAIR(FOCUS_COLOR));
    }
}

void update_mem(CORE* core) {
    WINDOW* win = win_base->mem_win;
    wclear(win);
    ADDR addr = win_base->mem_start;
    for (ADDR offset = 0; offset < 0x100; offset++) {
        wattron(win, COLOR_PAIR(SUBTITLE_COLOR));
        if (offset % 0x10 == 0)
            wprintw(win, "0x%08X     ", addr + offset);
        wattroff(win, COLOR_PAIR(SUBTITLE_COLOR));
        if ((addr + offset) / 0x10 == win_base->mem_focus / 0x10)
            wattron(win, COLOR_PAIR(FOCUS_COLOR));
        wprintw(win, " %02X", core->load(addr + offset, 0, 0));
        wattroff(win, COLOR_PAIR(FOCUS_COLOR));
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
    return wait4command();
}

void deinit() {
    // at the end of program
    // ncurses mode should be exited properly
    endwin();
}

void init_win(WIN* win) {
    win_base = win;
    char title[] = " RISC-V simulator ";

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
    init_pair(FOCUS_COLOR, COLOR_MAGENTA, COLOR_BLACK);
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
    // show basic info
    attron(A_STANDOUT);
    mvprintw(1, 1, title);
    attroff(A_STANDOUT);
    mvprintw(23, 72, " PENG AO ");
    // register inner windows to WIN
    win->pc_win = pc_inner;
    win->reg_win = reg_inner;
    win->mem_win = mem_inner;
    win->com_win = com_inner;

    win->reg_set = REG_SET_DEF;
    memset(win->reg_focus, 0, 32);
    win->mem_start = 0x0;
    win->mem_focus = 0xFFFFFFFF;

    win->update = update;
    win->deinit = deinit;

    show_splash_info();
}
