#include "gui.h"

void show_analysis_win(CORE* core) {
    clear();
    attron(COLOR_PAIR(TITLE_COLOR));
    box(stdscr, 0, 0);
    mvprintw(0, 2, " Analysis ");
    WINDOW* block1 = newwin(20, 35, 2, 2);
    mvvline(2, 38, ACS_VLINE, 20);
    WINDOW* block2 = newwin(10, 38, 2, 40);
    mvhline(12, 39, ACS_HLINE, 40);
    WINDOW* block3 = newwin(9, 38, 13, 40);
    attroff(COLOR_PAIR(TITLE_COLOR));
    // block1: instruction
    mvwprintw(block1, 0, 0, "RV32I & RV32M");
    for (int i = 0; i < 10; i++)
        mvwprintw(block1, i + 2, 0, "%-8s %8u", instr_name[i], core->instr_analysis[i]);
    mvwprintw(block1, 0, 18, "RV32F");
    for (int i = 10; i < 23; i++)
        mvwprintw(block1, i - 10 + 2, 18, "%-8s %8u", instr_name[i], core->instr_analysis[i]);
    mvwprintw(block1, 19, 0, "%u in total", core->instr_counter);
    // block2: cache info
    mvwprintw(block2, 0, 0, "%u way Set-associative Cache Info", ASSOCIATIVITY);
    mvwprintw(block2, 2, 0,
        #if defined(CACHE_FIFO)
        "policy: FIFO"
        #elif defined(CACHE_LRU)
        "policy: LRU"
        #elif defined(CACHE_RR)
        "policy: Round Robin"
        #else
        "policy: Alway 1st entry (default)"
        #endif
    );
    mvwprintw(block2, 3, 0, "addr = tag[%u] : idx[%u] : offset[%u]", TAG_LEN, SET_IDX_LEN, OFFSET_LEN);
    mvwprintw(block2, 4, 0, "cache size = %u block * 0x%X bytes", BLOCK_NUM, BLOCK_SIZE);
    mvwprintw(block2, 6, 0, "read  %u times", core->mmu->data_cache->read_counter);
    mvwprintw(block2, 7, 0, "write %u times", core->mmu->data_cache->write_counter);
    mvwprintw(block2, 8, 0, "hit   %u times", core->mmu->data_cache->hit_counter);
    mvwprintw(block2, 9, 0, "miss  %u times", core->mmu->data_cache->miss_counter);
    // block3: reserved
    mvwprintw(block3, 4, 15, "reserved");
    // refresh
    refresh();
    wrefresh(block1);
    wrefresh(block2);
    wrefresh(block3);
    getch();
}
