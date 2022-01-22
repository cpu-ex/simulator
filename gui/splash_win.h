#include "gui.h"

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
    delwin(splash_box);
}
