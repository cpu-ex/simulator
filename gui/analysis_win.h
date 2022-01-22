#include "gui.h"

void show_analysis_win(CORE* core) {
    clear();
    attron(COLOR_PAIR(TITLE_COLOR));
    box(stdscr, 0, 0);
    mvprintw(0, 2, " Analysis ");
    WINDOW* block1 = newwin(20, 39, 2, 2);
    mvvline(2, 42, ACS_VLINE, 20);
    WINDOW* block2 = newwin(10, 34, 2, 44);
    mvhline(12, 43, ACS_HLINE, 36);
    WINDOW* block3 = newwin(9, 34, 13, 44);
    attroff(COLOR_PAIR(TITLE_COLOR));
    // block1: instruction
    wattron(block1, COLOR_PAIR(STANDOUT_COLOR));
    mvwprintw(block1, 0, 0, "Instruction Counter Info:");
    wattroff(block1, COLOR_PAIR(STANDOUT_COLOR));
    mvwprintw(block1, 2, 0, "RV32I");
    for (int i = 0; i < 10; ++i)
        mvwprintw(block1, i + 3, 0, "%-7s %11llu", instr_name[i], core->instr_analysis[i]);
    mvwprintw(block1, 2, 20, "RV32F");
    for (int i = 10; i < 23; ++i)
        mvwprintw(block1, i - 10 + 3, 20, "%-7s %11llu", instr_name[i], core->instr_analysis[i]);
    mvwprintw(block1, 16, 0, "instruction(s): %llu", core->instr_counter);
    mvwprintw(block1, 17, 0, "cycle(s)      : %llu", core->instr_counter + core->stall_counter);
    mvwprintw(block1, 18, 0, "stall(s)      : %llu", core->stall_counter);
    mvwprintw(block1, 19, 0, "time in second: %.6lf", core->predict_exec_time(core));
    // block2: cache info
    wattron(block2, COLOR_PAIR(STANDOUT_COLOR));
    if (core->mmu->is_nocache) {
        mvwprintw(block2, 5, 4, "cache system not equipped");
    } else {
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
        mvwprintw(block2, 3, 0, "addr = tag[%u]+idx[%u]+offset[%u]", TAG_LEN, SET_IDX_LEN, OFFSET_LEN);
        mvwprintw(block2, 4, 0, "cache size = %u * %u * %u words", ASSOCIATIVITY, SET_NUM, BLOCK_SIZE);
        mvwprintw(block2, 6, 0, "read  %u times", core->mmu->data_cache->read_counter);
        mvwprintw(block2, 7, 0, "write %u times", core->mmu->data_cache->write_counter);
        u64 cacheHitTimes = core->mmu->data_cache->hit_counter;
        u64 cacheMissTimes = core->mmu->data_cache->miss_counter;
        f64 cacheTotal = (f64)(cacheHitTimes + cacheMissTimes);
        mvwprintw(block2, 8, 0, "hit   %u times (%.3lf%%)", cacheHitTimes, (cacheTotal > 0) ? ((f64)cacheHitTimes * 100 / cacheTotal) : 0);
        mvwprintw(block2, 9, 0, "miss  %u times (%.3lf%%)", cacheMissTimes, (cacheTotal > 0) ? ((f64)cacheMissTimes * 100 / cacheTotal) : 0);
    }
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
    u32 bpHitTimes = core->branch_predictor->hit_counter;
    u32 bpMissTimes = core->branch_predictor->miss_counter;
    f64 bpTotal = (f64)(bpHitTimes + bpMissTimes);
    mvwprintw(block3, 4, 0, "hit   %u times (%.3lf%%)", bpHitTimes, (bpTotal > 0) ? ((f64)bpHitTimes * 100 / bpTotal) : 0);
    mvwprintw(block3, 5, 0, "miss  %u times (%.3lf%%)", bpMissTimes, (bpTotal > 0) ? ((f64)bpMissTimes * 100 / bpTotal) : 0);
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
