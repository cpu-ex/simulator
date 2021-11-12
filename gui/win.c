#include "win.h"
#include "../src/instr.h"

#define TITLE_COLOR     1
#define SUBTITLE_COLOR  2
#define WARNING_COLOR   3
#define HIGHLIGHT_COLOR 4
#define STANDOUT_COLOR  5

static WIN* win_base;

void update_pc(WINDOW* win, CORE* core) {
    wclear(win);
    // fetch pc and op
    ADDR pc = core->pc;
    WORD op = core->load_instr(pc);
    // disasm
    char asm_buf[24];
    INSTR curr_instr = { .raw = op };
    disasm(curr_instr, asm_buf);
    // update
    wprintw(win, "%-8u 0x%08X : %08X : %-21s", core->instr_counter, pc, op, asm_buf);
    wattron(win, COLOR_PAIR(WARNING_COLOR));
    switch (BROADCAST.decoder.type) {
    case STAT_EXIT: wprintw(win, "%24s", "exit"); break;
    case STAT_HALT: wprintw(win, "%24s", "halt"); break;
    case STAT_STEP: wprintw(win, "%24s", "step"); break;
    case STAT_MEM_EXCEPTION: wprintw(win, "%24s", "mem exception"); break;
    case STAT_INSTR_EXCEPTION: wprintw(win, "%24s", "instr exception"); break;
    default: wprintw(win, "%24s", "quit or unknow"); break;
    }
    wattroff(win, COLOR_PAIR(WARNING_COLOR));
}

void update_reg(WINDOW* win, CORE* core) {
    wclear(win);

    int count = reg_set[win_base->reg_set][0];
    int* regs = reg_set[win_base->reg_set] + 1;
    for (int idx = 0; idx < count; idx++) {
        wattron(win, COLOR_PAIR(SUBTITLE_COLOR));
        if (regs[idx] == 8 && win_base->reg_set == REG_SET_DEF)
            wprintw(win, "fp   ");
        else if (regs[idx] < 32)
            wprintw(win, "%-4s ", reg_name[regs[idx]]);
        else
            wprintw(win, "%-4s ", freg_name[regs[idx] - 32]);
        wattroff(win, COLOR_PAIR(SUBTITLE_COLOR));
        if (win_base->reg_focus[regs[idx]])
            wattron(win, COLOR_PAIR(HIGHLIGHT_COLOR));
        if (regs[idx] < 32)
            wprintw(win, "%8X\n", core->regs[regs[idx]]);
        else
            wprintw(win, "%8.3f\n", *(float*)&(core->fregs[regs[idx] - 32]));
        wattroff(win, COLOR_PAIR(HIGHLIGHT_COLOR));
    }
}

void update_mem(WINDOW* win, CORE* core) {
    wclear(win);

    // handle memory access exception
    if (BROADCAST.decoder.type == STAT_MEM_EXCEPTION) {
        win_base->mem_start = BROADCAST.decoder.info & (~0xFF);
        win_base->mem_focus = BROADCAST.decoder.info;
        wattron(win, COLOR_PAIR(WARNING_COLOR));
        wprintw(win, "invalid access to 0x%08X", BROADCAST.decoder.info);
        wattroff(win, COLOR_PAIR(WARNING_COLOR));
        wgetch(win);
        wclear(win);
    }

    ADDR addr = win_base->mem_start;
    BYTE (*load)(ADDR) = win_base->mem_type ? core->mmu->read_instr : core->mmu->read_data;
    for (ADDR offset = 0; offset < 0x100; offset++) {
        wattron(win, COLOR_PAIR(SUBTITLE_COLOR));
        if (offset % 0x10 == 0)
            wprintw(win, "0x%08X   ", addr + offset);
        wattroff(win, COLOR_PAIR(SUBTITLE_COLOR));
        if (((addr + offset) & (~0xF)) == (win_base->mem_focus & (~0xF)))
            wattron(win, COLOR_PAIR(HIGHLIGHT_COLOR));
        if (win_base->mem_type && ((addr + offset) & (~0x3)) == core->pc)
            wattron(win, COLOR_PAIR(STANDOUT_COLOR));
        wprintw(win, " %02X", load(addr + offset));
        wattroff(win, COLOR_PAIR(HIGHLIGHT_COLOR));
        wattroff(win, COLOR_PAIR(STANDOUT_COLOR));
        if (offset % 0x10 == 0xF)
            wprintw(win, "\n");
    }
}

void show_main_win(CORE* core) {
    // create outer windows (containers that never change)
    WINDOW* pc_outer = newwin(3, 80, 0, 0);
    WINDOW* reg_outer = newwin(18, 16, 3, 0);
    WINDOW* mem_outer = newwin(18, 64, 3, 16);
    WINDOW* com_outer = newwin(3, 80, 21, 0);
    // box the containers
    wattron(pc_outer, COLOR_PAIR(TITLE_COLOR));
    wattron(reg_outer, COLOR_PAIR(TITLE_COLOR));
    wattron(mem_outer, COLOR_PAIR(TITLE_COLOR));
    wattron(com_outer, COLOR_PAIR(TITLE_COLOR));
    box(pc_outer, 0, 0); mvwprintw(pc_outer, 0, 2, " PC "); wrefresh(pc_outer);
    box(reg_outer, 0, 0); mvwprintw(reg_outer, 0, 2, " Register "); wrefresh(reg_outer);
    box(mem_outer, 0, 0); mvwprintw(mem_outer, 0, 2, win_base->mem_type ? " Memory: instruction " : " Memory: data "); wrefresh(mem_outer);
    box(com_outer, 0, 0); mvwprintw(com_outer, 0, 2, " Input "); wrefresh(com_outer);
    wattroff(pc_outer, COLOR_PAIR(TITLE_COLOR));
    wattroff(reg_outer, COLOR_PAIR(TITLE_COLOR));
    wattron(mem_outer, COLOR_PAIR(TITLE_COLOR));
    wattron(com_outer, COLOR_PAIR(TITLE_COLOR));
    // create inner windows (contents that actually display unpdates)
    WINDOW* pc_inner = newwin(1, 78, 1, 1);
    WINDOW* reg_inner = newwin(16, 14, 4, 1);
    WINDOW* mem_inner = newwin(16, 62, 4, 17);
    WINDOW* com_inner = newwin(1, 77, 22, 2);
    // update contents
    update_pc(pc_inner, core);
    update_reg(reg_inner, core);
    update_mem(mem_inner, core);
    // refresh
    refresh();
    wrefresh(pc_inner);
    wrefresh(reg_inner);
    wrefresh(mem_inner);
}

void show_analysis_win(CORE* core) {
    clear();
    attron(COLOR_PAIR(TITLE_COLOR));
    box(stdscr, 0, 0);
    mvprintw(0, 2, " Analysis ");
    WINDOW* block1 = newwin(20, 35, 2, 2);
    mvvline(2, 38, ACS_VLINE, 20);
    WINDOW* block2 = newwin(20, 38, 2, 40);
    attroff(COLOR_PAIR(TITLE_COLOR));
    // block1: instruction
    mvwprintw(block1, 0, 0, "RV32I & RV32M");
    for (int i = 0; i < 10; i++)
        mvwprintw(block1, i + 2, 0, "%-8s %8u", instr_name[i], core->instr_analysis[i]);
    mvwprintw(block1, 0, 18, "RV32F");
    for (int i = 10; i < 23; i++)
        mvwprintw(block1, i - 10 + 2, 18, "%-8s %8u", instr_name[i], core->instr_analysis[i]);
    mvwprintw(block1, 19, 0, "%u in total", core->instr_counter);
    // block2: reserved
    mvwprintw(block2, 9, 15, "reserved");
    // refresh
    refresh();
    wrefresh(block1);
    wrefresh(block2);
    getch();
}

void show_help_win() {
    clear();
    WINDOW* help_win_outer = newwin(20, 70, 2, 5);
    WINDOW* help_win_inner = newwin(18, 68, 3, 6);
    wattron(help_win_outer, COLOR_PAIR(TITLE_COLOR));
    box(help_win_outer, 0, 0);
    mvwprintw(help_win_outer, 0, 2, " Instruction ");
    wattroff(help_win_outer, COLOR_PAIR(TITLE_COLOR));
    mvwprintw(help_win_inner, 0, 3, "step [n]:\n\tmove on for n step,\n\tpositive for forward, negative for backwards\n\tdefault to infinity (loops util exit or exception)");
    mvwprintw(help_win_inner, 4, 3, "reg [d|a|s|t|fa|fs|ft]:\n\tswitch register set, [d]default to zero ~ a5");
    mvwprintw(help_win_inner, 6, 3, "reg [-][reg name]:\n\thighlight certain register,\n\tminus for setting back to normal,\n\tmutiple input supported");
    mvwprintw(help_win_inner, 10, 3, "instr|data [address(hex)]:\n\tswitch memory range, example: instr 0x100");
    mvwprintw(help_win_inner, 12, 3, "analysis:\n\tshow analysis window");
    mvwprintw(help_win_inner, 14, 3, "help:\n\tshow help window");
    mvwprintw(help_win_inner, 16, 3, "quit:\n\texit simulator");
    refresh();
    wrefresh(help_win_outer);
    wgetch(help_win_inner);
}

void show_splash_win() {
    WINDOW* splash_box = newwin(6, 40, 6, 20);
    box(splash_box, 0, 0);
    char title[] = "RISC-V simulator";
    char author[] = "PENG AO";
    char info[] = "-> press any key but 'h' to skip <-";
    mvwprintw(splash_box, 2, (40 - strlen(title)) / 2, title);
    mvwprintw(splash_box, 3, (40 - strlen(author)) / 2, author);
    attron(COLOR_PAIR(WARNING_COLOR));
    mvprintw(13, (80 - strlen(info)) / 2, info);
    attroff(COLOR_PAIR(WARNING_COLOR));
    refresh();
    wrefresh(splash_box);
    // catch char h
    if (getch() == 'h') {
        show_help_win();
    }
}

typedef struct command {
    char type;
    int argc;
    int argv[10];
} COMMAND;

int reg2idx(char* reg) {
    // special fp
    if (!strcmp(reg, "fp"))
        return 8;
    // try regs and fregs
    for (int idx = 0; idx < 32; idx++) {
        if (!strcmp(reg, reg_name[idx]))
            return idx;
        if (!strcmp(reg, freg_name[idx]))
            return 32 + idx;
    }
    // convert d|a|s|t|fa|fs|ft into a int number
    int val = 0;
    for (int idx = 0; idx < strlen(reg); idx++)
        val += reg[idx];
    return val;
}

COMMAND get_command() {
    // echo + blinking cursor
    echo();
    curs_set(1);
    // prepare a COMMAND struct
    COMMAND com;
    com.argc = 0;
    // get instruction
    char input[73], output[12][12];
    WINDOW* com_win = newwin(1, 77, 22, 2);
    wclear(com_win);
    mvwgetstr(com_win, 0, 0, input);
    // split with space (exactly 1 space)
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
        com.argv[0] = argc > 1 ? atoi(output[1]) : 0x7FFFFFFF;
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
                com.argv[i - 1] = flag * idx;
            }
        } else {
            com.argc = 1;
            com.argv[0] = 'd'; // default reg set
        }
    } else if (!strcmp(output[0], "instr")) {
        com.type = 'm';
        com.argv[0] = 'i';
        if (argc > 1) {
            com.argc = 2;
            sscanf(output[1], "0x%X", com.argv + 1);
            if (!com.argv[1]) // not recognizable
                com.argc = 1;
        } else {
            com.argc = 1;
        }
    } else if (!strcmp(output[0], "data")) {
        com.type = 'm';
        com.argv[0] = 'd';
        if (argc > 1) {
            com.argc = 2;
            sscanf(output[1], "0x%X", com.argv + 1);
            if (!com.argv[0]) // not recognizable
                com.argc = 1;
        } else {
            com.argc = 1;
        }
    } else if (!strcmp(output[0], "help")) {
        com.type = 'h';
    } else if (!strcmp(output[0], "analysis")) {
        com.type = 'a';
    } else {
        // parse as step 1
        com.type = 's';
        com.argc = 1;
        com.argv[0] = 1;
    }
    // noecho + hide cursor
    noecho();
    curs_set(0);
    return com;
}

STATE wait4command(CORE* core) {
    COMMAND com = get_command();
    switch (com.type) {
    case 'q':
        return STAT_QUIT;
    case 's':
        return STAT_STEP | ((u64)com.argv[0] << 32);
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
            } else if (arg == 'f' + 'a') {
                win_base->reg_set = REG_SET_FA;
            } else if (arg == 'f' + 's') {
                win_base->reg_set = REG_SET_FS;
            } else if (arg == 'f' + 't') {
                win_base->reg_set = REG_SET_FT;
            } else if (64 > arg && arg > -64) {
                if (arg > 0)
                    win_base->reg_focus[arg] = 1;
                else
                    win_base->reg_focus[-arg] = 0;
            }
        }
        return STAT_HALT;
    case 'm':
        win_base->mem_type = com.argv[0] == 'i' ? MEM_INSTR : MEM_DATA;
        win_base->mem_start = com.argc > 1 ? com.argv[1] & (~0xFF) : (win_base->mem_type ? 0x100 : 0);
        win_base->mem_focus = com.argc > 1 ? com.argv[1] : 0xFFFFFFFF;
        return STAT_HALT;
    case 'a':
        show_analysis_win(core);
        return STAT_HALT;
    case 'h':
        show_help_win();
        return STAT_HALT;
    default:
        return STAT_HALT;
    }
}

STATE update(CORE* core) {
    show_main_win(core);
    // wait for a new command
    return wait4command(core);
}

void deinit() {
    // at the end of program ncurses mode should be exited properly
    endwin();
}

void init_win(WIN* win) {
    win_base = win;
    char title[] = " RISC-V simulator ";
    // resize the terminal to 80 * 24
    printf("\e[8;24;80t");
    // initialize ncurses mode, and end it in deinit()
    initscr();
    noecho();
    curs_set(0);
    // set colors if supported
    if (has_colors()) start_color();
    init_pair(TITLE_COLOR, COLOR_BLUE, COLOR_BLACK);
    init_pair(SUBTITLE_COLOR, COLOR_CYAN, COLOR_BLACK);
    init_pair(WARNING_COLOR, COLOR_RED, COLOR_BLACK);
    init_pair(HIGHLIGHT_COLOR, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(STANDOUT_COLOR, COLOR_YELLOW, COLOR_BLACK);
    // regist variables
    win->reg_set = REG_SET_DEF;
    memset(win->reg_focus, 0, 32);
    win->mem_type = MEM_INSTR;
    win->mem_start = DEFAULT_PC;
    win->mem_focus = 0xFFFFFFFF;
    // assign interfaces
    win->update = update;
    win->deinit = deinit;

    show_splash_win();
}
