#include "gui.h"

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
