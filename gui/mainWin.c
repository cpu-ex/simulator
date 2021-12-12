#include "mainWin.h"

void update_pc(WINDOW* outer, WINDOW* inner, GUI* gui, CORE* core) {
    wattron(outer, COLOR_PAIR(TITLE_COLOR));
    box(outer, 0, 0); mvwprintw(outer, 0, 2, " PC ");
    wattroff(outer, COLOR_PAIR(TITLE_COLOR));

    // fetch pc and op
    ADDR pc = core->pc;
    WORD op = core->load_instr(core, pc);
    // disasm
    char asm_buf[24];
    INSTR instr = { .raw = op };
    disasm(instr, asm_buf);
    // render
    wprintw(inner, "%-10u 0x%08X : %08X : %-19s", core->instr_counter, pc, op, asm_buf);
    wattron(inner, COLOR_PAIR(WARNING_COLOR));
    switch (BROADCAST.decoder.type) {
    case STAT_EXIT: wprintw(inner, "%24s", "exit"); break;
    case STAT_HALT: wprintw(inner, "%24s", "halt"); break;
    case STAT_STEP: wprintw(inner, "%24s", "step"); break;
    case STAT_MEM_EXCEPTION: wprintw(inner, "%24s", "mem exception"); break;
    case STAT_INSTR_EXCEPTION: wprintw(inner, "%24s", "instr exception"); break;
    default: wprintw(inner, "%24s", "quit or unknow"); break;
    }
    wattroff(inner, COLOR_PAIR(WARNING_COLOR));

    wrefresh(outer);
    wrefresh(inner);
}

void update_reg_sub(WINDOW* win, GUI* gui, CORE* core, u8 focused) {
    wclear(win);
    for (int idx = gui->reg_start; idx < min(gui->reg_start + 16, 64); idx++) {
        // print reg name
        wattron(win, COLOR_PAIR(SUBTITLE_COLOR));
        mvwprintw(win, idx - gui->reg_start, 0, "%-4s", (idx < 32) ? reg_name[idx] : freg_name[idx - 32]);
        wattroff(win, COLOR_PAIR(SUBTITLE_COLOR));
        // print reg value
        if (focused && idx == gui->reg_start) wattron(win, A_STANDOUT);
        if (gui->reg_focus[idx]) wattron(win, COLOR_PAIR(HIGHLIGHT_COLOR));
        if (idx < 32)
            wprintw(win, "%8X", core->regs[idx]);
        else
            wprintw(win, "%8.3f", *(float*)&(core->fregs[idx - 32]));
        wattroff(win, A_STANDOUT);
        wattroff(win, COLOR_PAIR(HIGHLIGHT_COLOR));
    }
}

void update_reg(WINDOW* outer, WINDOW* inner, GUI* gui, CORE* core) {
    if (gui->focused_win == REG_WIN) wattron(outer, COLOR_PAIR(STANDOUT_COLOR));
    else wattron(outer, COLOR_PAIR(TITLE_COLOR));
    box(outer, 0, 0); mvwprintw(outer, 0, 2, " Register ");
    wattroff(outer, COLOR_PAIR(TITLE_COLOR));
    wattroff(outer, COLOR_PAIR(STANDOUT_COLOR));

    // render
    if (gui->focused_win == REG_WIN) {
        // focused
        keypad(stdscr, 1);
        while (gui->focused_win == REG_WIN) {
            // update
            gui->reg_start = max(0, min(gui->reg_start, 63));
            mvwprintw(outer, 1, 7, (gui->reg_start > 0) ? "^" : " ");
            mvwprintw(outer, 18, 7, (gui->reg_start < 63) ? "v" : " ");
            update_reg_sub(inner, gui, core, 1);
            wrefresh(outer);
            wrefresh(inner);
            // interaction
            switch (getch()) {
            case KEY_UP: gui->reg_start -= 1; break;
            case KEY_DOWN: gui->reg_start += 1; break;
            case KEY_LEFT: gui->reg_start -= 16; break;
            case KEY_RIGHT: gui->reg_start += 16; break;
            case '\n': gui->reg_focus[gui->reg_start] = gui->reg_focus[gui->reg_start] ? 0 : 1; break;
            default: keypad(stdscr, 0); gui->focused_win = COM_WIN; break;
            }
        }
    } else {
        // not focused
        update_reg_sub(inner, gui, core, 0);
    }

    wrefresh(outer);
    wrefresh(inner);
}

void update_mem_sub(WINDOW* win, GUI* gui, CORE* core, u8 focused) {
    wclear(win);
    for (int i = gui->mem_start; i < min(gui->mem_start + 16, MAX_ADDR >> 4); i++) {
        wattron(win, COLOR_PAIR(SUBTITLE_COLOR));
        mvwprintw(win, i - gui->mem_start, 0, "0x%08X   ", i << 4);
        wattroff(win, COLOR_PAIR(SUBTITLE_COLOR));
        for (int j = 0; j < 0x10; j++) {
            if (gui->mem_type && (((i << 4) + j) & (~0x3)) == core->pc)
                wattron(win, COLOR_PAIR(STANDOUT_COLOR));
            wprintw(win, " %02X", core->mmu->sneak(core->mmu, (i << 4) + j, gui->mem_type));
            wattroff(win, COLOR_PAIR(STANDOUT_COLOR));
        }
    }
}

void update_mem(WINDOW* outer, WINDOW* inner, GUI* gui, CORE* core) {
    if (gui->focused_win == MEM_WIN) wattron(outer, COLOR_PAIR(STANDOUT_COLOR));
    else wattron(outer, COLOR_PAIR(TITLE_COLOR));
    box(outer, 0, 0); mvwprintw(outer, 0, 2, gui->mem_type ? " Memory: instruction " : " Memory: data ");
    wattroff(outer, COLOR_PAIR(TITLE_COLOR));
    wattroff(outer, COLOR_PAIR(STANDOUT_COLOR));

    // handle memory accsess exception
    if (BROADCAST.decoder.type == STAT_MEM_EXCEPTION) {
        wattron(inner, COLOR_PAIR(WARNING_COLOR));
        wprintw(inner, "invalid access to 0x%08X", BROADCAST.decoder.info);
        wattroff(inner, COLOR_PAIR(WARNING_COLOR));
        wgetch(inner);
        wclear(inner);
    }

    // render
    if (gui->focused_win == MEM_WIN) {
        // focused
        keypad(stdscr, 1);
        while (gui->focused_win == MEM_WIN) {
            // update
            gui->mem_start = max(0, min((signed)gui->mem_start, (MAX_ADDR >> 4) - 1));
            mvwprintw(outer, 1, 31, (gui->mem_start > 0) ? "^" : " ");
            mvwprintw(outer, 18, 31, (gui->mem_start < (MAX_ADDR >> 4) - 1) ? "v" : " ");
            update_mem_sub(inner, gui, core, 1);
            wrefresh(outer);
            wrefresh(inner);
            // interaction
            switch (getch()) {
            case KEY_UP: gui->mem_start -= 1; break;
            case KEY_DOWN: gui->mem_start += 1; break;
            case KEY_LEFT: gui->mem_start -= 16; break;
            case KEY_RIGHT: gui->mem_start += 16; break;
            default: keypad(stdscr, 0); gui->focused_win = COM_WIN; break;
            }
        }
    } else {
        // not focused
        update_mem_sub(inner, gui, core, 0);
    }

    wrefresh(outer);
    wrefresh(inner);
}

void update_com(WINDOW* outer, WINDOW* inner, GUI* gui, CORE* core) {
    if (gui->focused_win == COM_WIN) wattron(outer, COLOR_PAIR(STANDOUT_COLOR));
    else wattron(outer, COLOR_PAIR(TITLE_COLOR));
    box(outer, 0, 0); mvwprintw(outer, 0, 2, " Input ");
    wattroff(outer, COLOR_PAIR(TITLE_COLOR));
    wattroff(outer, COLOR_PAIR(STANDOUT_COLOR));
    wrefresh(outer);
}

void show_main_win(GUI* gui, CORE* core) {
    // create outer windows
    WINDOW* pc_outer = newwin(3, 80, 0, 0);
    WINDOW* pc_inner = newwin(1, 78, 1, 1);
    WINDOW* reg_outer = newwin((gui->focused_win == REG_WIN) ? 20 : 18, 16, 3, 0);
    WINDOW* reg_inner = newwin(16, 14, (gui->focused_win == REG_WIN) ? 5 : 4, 1);
    WINDOW* mem_outer = newwin((gui->focused_win == MEM_WIN) ? 20 : 18, 64, 3, 16);
    WINDOW* mem_inner = newwin(16, 62, (gui->focused_win == MEM_WIN) ? 5 : 4, 17);
    WINDOW* com_outer = newwin(3, 80, 21, 0);
    // refresh
    refresh();
    update_pc(pc_outer, pc_inner, gui, core);
    update_com(com_outer, NULL, gui, core);
    update_reg(reg_outer, reg_inner, gui, core);
    update_mem(mem_outer, mem_inner, gui, core);
    // clean up
    delwin(pc_outer);
    delwin(reg_outer);
    delwin(mem_outer);
    delwin(com_outer);
    delwin(pc_inner);
    delwin(reg_inner);
    delwin(mem_inner);
}
