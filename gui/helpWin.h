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
        "step [n]: move on for n step, positive for forward, negative for\n\
        backwards, default to infinity (loops util exit or exception)");
    mvwprintw(help_win_inner, 2, 0,
        "dump [n]: move on for n step with register value recorded,\n\
        negative number not supported, default to infinity");
    mvwprintw(help_win_inner, 4, 0,
        "auto [ms]: start auto-stepping with interval ms, e.g. 1000 for 1sec,\n\
        meanwhile every commands are functional,\n\
        exit auto-stepping mode by typing \"stop\"");
    mvwprintw(help_win_inner, 7, 0,
        "reg [reg name]: focus register window, move contents with\n\
        arrow key, direct to certain register by giving its name,\n\
        set and cancel highlight with KEY_ENTER\n\
        exit focusing mode by pressing any key");
    mvwprintw(help_win_inner, 11, 0,
        "instr: display instruction window without focusing");
    mvwprintw(help_win_inner, 12, 0,
        "data [address(Hex)]: focus memory window, move contents with\n\
        arrow key, direct to certain address by giving a Hex number\n\
        exit focusing mode by pressing any key");
    mvwprintw(help_win_inner, 15, 0,
        "cache [setIdx]: display cache window, move contents with arrow key,\n\
        direct to certain block by giving its set index,\n\
        exit focusing mode by pressing any key");
    mvwprintw(help_win_inner, 18, 0, "analysis: display analysis window and press any key to close");
    mvwprintw(help_win_inner, 19, 0, "help: display help window and press any key to close");
    mvwprintw(help_win_inner, 20, 0, "quit: exit simulator");
    refresh();
    wrefresh(help_win_outer);
    wgetch(help_win_inner);
    // delete window pointers
    delwin(help_win_outer);
    delwin(help_win_inner);
}
