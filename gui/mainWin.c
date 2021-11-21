#include "mainWin.h"

void update_pc(GUI* gui, WINDOW* win, CORE* core) {
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

void update_reg(GUI* gui, WINDOW* win, CORE* core) {
    wclear(win);

    int count = reg_set[gui->reg_set][0];
    int* regs = reg_set[gui->reg_set] + 1;
    for (int idx = 0; idx < count; idx++) {
        wattron(win, COLOR_PAIR(SUBTITLE_COLOR));
        if (regs[idx] == 8 && gui->reg_set == REG_SET_DEF)
            wprintw(win, "fp   ");
        else if (regs[idx] < 32)
            wprintw(win, "%-4s ", reg_name[regs[idx]]);
        else
            wprintw(win, "%-4s ", freg_name[regs[idx] - 32]);
        wattroff(win, COLOR_PAIR(SUBTITLE_COLOR));
        if (gui->reg_focus[regs[idx]])
            wattron(win, COLOR_PAIR(HIGHLIGHT_COLOR));
        if (regs[idx] < 32)
            wprintw(win, "%8X\n", core->regs[regs[idx]]);
        else
            wprintw(win, "%8.3f\n", *(float*)&(core->fregs[regs[idx] - 32]));
        wattroff(win, COLOR_PAIR(HIGHLIGHT_COLOR));
    }
}

void update_mem(GUI* gui, WINDOW* win, CORE* core) {
    wclear(win);

    // handle memory access exception
    if (BROADCAST.decoder.type == STAT_MEM_EXCEPTION) {
        gui->mem_start = BROADCAST.decoder.info & (~0xFF);
        gui->mem_focus = BROADCAST.decoder.info;
        wattron(win, COLOR_PAIR(WARNING_COLOR));
        wprintw(win, "invalid access to 0x%08X", BROADCAST.decoder.info);
        wattroff(win, COLOR_PAIR(WARNING_COLOR));
        wgetch(win);
        wclear(win);
    }

    ADDR addr = gui->mem_start;
    // BYTE (*load)(ADDR) = gui->mem_type ? core->mmu->read_instr : core->mmu->read_data;
    for (ADDR offset = 0; offset < 0x100; offset++) {
        wattron(win, COLOR_PAIR(SUBTITLE_COLOR));
        if (offset % 0x10 == 0)
            wprintw(win, "0x%08X   ", addr + offset);
        wattroff(win, COLOR_PAIR(SUBTITLE_COLOR));
        if (((addr + offset) & (~0xF)) == (gui->mem_focus & (~0xF)))
            wattron(win, COLOR_PAIR(HIGHLIGHT_COLOR));
        if (gui->mem_type && ((addr + offset) & (~0x3)) == core->pc)
            wattron(win, COLOR_PAIR(STANDOUT_COLOR));
        wprintw(win, " %02X", core->mmu->sneak(addr + offset, gui->mem_type));
        wattroff(win, COLOR_PAIR(HIGHLIGHT_COLOR));
        wattroff(win, COLOR_PAIR(STANDOUT_COLOR));
        if (offset % 0x10 == 0xF)
            wprintw(win, "\n");
    }
}

void show_main_win(GUI* gui, CORE* core) {
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
    box(mem_outer, 0, 0); mvwprintw(mem_outer, 0, 2, gui->mem_type ? " Memory: instruction " : " Memory: data "); wrefresh(mem_outer);
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
    update_pc(gui, pc_inner, core);
    update_reg(gui, reg_inner, core);
    update_mem(gui, mem_inner, core);
    // refresh
    refresh();
    wrefresh(pc_inner);
    wrefresh(reg_inner);
    wrefresh(mem_inner);
}
