#include "gui.h"

void show_help_win() {
    clear();
    WINDOW* help_win_outer = newwin(22, 70, 1, 5);
    WINDOW* help_win_inner = newwin(20, 68, 2, 6);
    wattron(help_win_outer, COLOR_PAIR(TITLE_COLOR));
    box(help_win_outer, 0, 0);
    mvwprintw(help_win_outer, 0, 2, " Instruction ");
    wattroff(help_win_outer, COLOR_PAIR(TITLE_COLOR));
    mvwprintw(help_win_inner, 0, 3, "step [n]:\n\tmove on for n step,\n\tpositive for forward, negative for backwards\n\tdefault to infinity (loops util exit or exception)");
    mvwprintw(help_win_inner, 4, 3, "reg [d|a|s|t|fa|fs|ft]:\n\tswitch register set, [d]default to zero ~ a5");
    mvwprintw(help_win_inner, 6, 3, "reg [-][reg name]:\n\thighlight certain register,\n\tminus for setting back to normal,\n\tmutiple input supported");
    mvwprintw(help_win_inner, 10, 3, "instr|data [address(hex)]:\n\tswitch memory range, example: instr 0x100");
    mvwprintw(help_win_inner, 12, 3, "analysis:\n\tdisplay analysis window and press any button to close");
    mvwprintw(help_win_inner, 14, 3, "cache:\n\tdisplay cache window and control with arrow key intuitively");
    mvwprintw(help_win_inner, 16, 3, "help:\n\tdisplay help window and press any button to close");
    mvwprintw(help_win_inner, 18, 3, "quit:\n\texit simulator");
    refresh();
    wrefresh(help_win_outer);
    wgetch(help_win_inner);
    // delete window pointers
    delwin(help_win_outer);
    delwin(help_win_inner);
}
