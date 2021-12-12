#include "gui.h"

void show_help_win() {
    clear();
    WINDOW* help_win_outer = newwin(24, 80, 0, 0);
    WINDOW* help_win_inner = newwin(22, 74, 1, 3);
    wattron(help_win_outer, COLOR_PAIR(TITLE_COLOR));
    box(help_win_outer, 0, 0);
    mvwprintw(help_win_outer, 0, 2, " Instruction ");
    wattroff(help_win_outer, COLOR_PAIR(TITLE_COLOR));
    mvwprintw(help_win_inner, 0, 0,
        "step [n]:\n\
        move on for n step, positive for forward, negative for backwards\n\
        default to infinity (loops util exit or exception)");
    mvwprintw(help_win_inner, 3, 0,
        "auto [ms]\n\
        start auto-stepping with interval ms, 1000 for 1sec\n\
        meanwhile every commands are functional\n\
        exit auto-stepping mode by typing \"stop\"");
    mvwprintw(help_win_inner, 7, 0,
        "reg [reg name]:\n\
        focus register window, move contents with arrow key\n\
        direct to certain register by giving its name\n\
        set and cancel highlight with KEY_ENTER\n\
        exit focusing mode by pressing any key");
    mvwprintw(help_win_inner, 12, 0,
        "instr|data [address(Hex)]:\n\
        focus memory window, move contents with arrow key\n\
        direct to certain address by giving a Hex number\n\
        exit focusing mode by pressing any key");
    mvwprintw(help_win_inner, 16, 0,
        "cache:\n\
        display cache window, move contents with arrow key\n\
        exit focusing mode by pressing any key");
    mvwprintw(help_win_inner, 19, 0, "analysis: display analysis window and press any key to close");
    mvwprintw(help_win_inner, 20, 0, "help:\tdisplay help window and press any key to close");
    mvwprintw(help_win_inner, 21, 0, "quit:\texit simulator");
    refresh();
    wrefresh(help_win_outer);
    wgetch(help_win_inner);
    // delete window pointers
    delwin(help_win_outer);
    delwin(help_win_inner);
}
