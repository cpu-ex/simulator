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
    wattron(block1, COLOR_PAIR(STANDOUT_COLOR));
    mvwprintw(block1, 0, 0, "Instruction Counter Info:");
    wattroff(block1, COLOR_PAIR(STANDOUT_COLOR));
    mvwprintw(block1, 2, 0, "RV32I & RV32M");
    for (int i = 0; i < 10; i++)
        mvwprintw(block1, i + 3, 0, "%-8s %8u", instr_name[i], core->instr_analysis[i]);
    mvwprintw(block1, 2, 18, "RV32F");
    for (int i = 10; i < 23; i++)
        mvwprintw(block1, i - 10 + 3, 18, "%-8s %8u", instr_name[i], core->instr_analysis[i]);
    mvwprintw(block1, 16, 0, "instruction(s): %u", core->instr_counter);
    u64 cycles = core->instr_counter + core->stall_counter;
    mvwprintw(block1, 17, 0, "cycle(s)      : %llu", cycles, core->stall_counter);
    mvwprintw(block1, 18, 0, "stall(s)      : %llu", core->stall_counter);
    mvwprintw(block1, 19, 0, "time in second: %.6f", (f32)cycles / CLK_FREQUENCY);
    // block2: cache info
    wattron(block2, COLOR_PAIR(STANDOUT_COLOR));
    mvwprintw(block2, 0, 0, "%u way Set-associative Cache Info:", ASSOCIATIVITY);
    wattroff(block2, COLOR_PAIR(STANDOUT_COLOR));
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
    // block3: branch predictor
    wattron(block3, COLOR_PAIR(STANDOUT_COLOR));
    mvwprintw(block3, 0, 0, "Branch Predictor Info:");
    wattroff(block3, COLOR_PAIR(STANDOUT_COLOR));
    mvwprintw(block3, 2, 0,
        #if defined(BP_AT)
        "policy: Always Taken"
        #elif defined(BP_NT)
        "policy: Always Untaken"
        #elif defined(BP_2BIT)
        "policy: 2-bit counter"
        #elif defined(BP_BIMODAL)
        "policy: Bimodal (PHT size = %u)", PHT_SIZE
        #elif defined(BP_GSHARE)
        "policy: Gshare (PHT size = %u)", PHT_SIZE
        #else
        "policy: Always Untaken (default)"
        #endif
    );
    mvwprintw(block3, 4, 0, "hit   %u times", core->branch_predictor->hit_counter);
    mvwprintw(block3, 5, 0, "miss  %u times", core->branch_predictor->miss_counter);
    // refresh
    refresh();
    wrefresh(block1);
    wrefresh(block2);
    wrefresh(block3);
    getch();
    // delete window pointers
    delwin(block1);
    delwin(block2);
    delwin(block3);
}
