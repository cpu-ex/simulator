#include "main_win.h"

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
    wprintw(inner, "%-11llu 0x%08X : %08X : %-24s", core->instr_counter, pc, op, asm_buf);
    wattron(inner, COLOR_PAIR(WARNING_COLOR));
    switch (BROADCAST.decoder.type) {
    case STAT_EXIT: wprintw(inner, "%18s", "exit"); break;
    case STAT_HALT: wprintw(inner, "%18s", "halt"); break;
    case STAT_STEP: wprintw(inner, "%18s", "step"); break;
    case STAT_MEM_EXCEPTION: wprintw(inner, "%18s", "mem exception"); break;
    case STAT_INSTR_EXCEPTION: wprintw(inner, "%18s", "instr exception"); break;
    default: wprintw(inner, "%18s", "quit or unknow"); break;
    }
    wattroff(inner, COLOR_PAIR(WARNING_COLOR));

    wrefresh(outer);
    wrefresh(inner);
}

void update_reg_sub(WINDOW* win, GUI* gui, CORE* core, u8 focused) {
    wclear(win);
    int start_point = gui->reg_start & ~0xF;
    for (int idx = start_point; idx < min(start_point + 16, 64); ++idx) {
        // print reg name
        wattron(win, COLOR_PAIR(SUBTITLE_COLOR));
        mvwprintw(win, idx - start_point, 0, "%-4s ", (idx < 32) ? reg_name[idx] : freg_name[idx - 32]);
        wattroff(win, COLOR_PAIR(SUBTITLE_COLOR));
        // print reg value
        if (focused && idx == gui->reg_start) wattron(win, A_STANDOUT);
        if (gui->reg_focus[idx]) wattron(win, COLOR_PAIR(HIGHLIGHT_COLOR));
        if (idx < 32) {
            u32 val = core->regs[idx];
            if (focused)
                wprintw(win, "0x%08X %11u %11d", val, val, val);
            else
                wprintw(win, "%8X", val);
        } else {
            u32 val = core->fregs[idx - 32];
            if (focused)
                wprintw(win, "0x%08X %23.8f", val, *(f32*)&val);
            else
                wprintw(win, "%8X", val);
        }
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
            mvwprintw(outer, 1, 2, (gui->reg_start > 0) ? "^" : " ");
            mvwprintw(outer, 18, 2, (gui->reg_start < 63) ? "v" : " ");
            mvwprintw(outer, 1, 13, (gui->reg_start < 32) ? "hex    unsigned      signed" : "hex                   float");
            update_reg_sub(inner, gui, core, 1);
            wrefresh(outer);
            wrefresh(inner);
            // interaction
            switch (getch()) {
            case KEY_UP: gui->reg_start -= 1; break;
            case KEY_DOWN: gui->reg_start += 1; break;
            case KEY_LEFT: gui->reg_start -= 16; break;
            case KEY_RIGHT: gui->reg_start += 16; break;
            case '\n': gui->reg_focus[gui->reg_start] = !gui->reg_focus[gui->reg_start]; break;
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
    if (gui->mem_type == MEM_INSTR) {
        gui->mem_start = (core->pc >> 2) & (~0xF);
        for (int i = gui->mem_start; i < min(gui->mem_start + 16, core->mmu->instr_len); ++i) {
            wattron(win, COLOR_PAIR(SUBTITLE_COLOR));
            mvwprintw(win, i - gui->mem_start, 0, "0x%08X ", i << 2);
            wattroff(win, COLOR_PAIR(SUBTITLE_COLOR));
            INSTR instr = { .raw = core->mmu->read_instr(core->mmu, i) };
            if (i == core->pc >> 2) wattron(win, COLOR_PAIR(STANDOUT_COLOR));
            char asmText[36]; disasm(instr, asmText);
            wprintw(win, "%08X     %-36s", instr.raw, asmText);
            wattroff(win, COLOR_PAIR(STANDOUT_COLOR));
        }
    } else {
        for (int i = gui->mem_start; i < min(gui->mem_start + 16, MAX_ADDR >> 4); ++i) {
            wattron(win, COLOR_PAIR(SUBTITLE_COLOR));
            mvwprintw(win, i - gui->mem_start, 0, "0x%08X   ", i << 4);
            wattroff(win, COLOR_PAIR(SUBTITLE_COLOR));
            for (int j = 0; j < 0x10; ++j)
                wprintw(win, " %02X", core->mmu->sneak(core->mmu, (i << 4) + j, gui->mem_type));
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
            gui->mem_start = max(0, min((s32)gui->mem_start, (MAX_ADDR >> 4) - 1));
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
    WINDOW* reg_outer = newwin(
        (gui->focused_win == REG_WIN) ? 20 : 18, (gui->focused_win == REG_WIN) ? 42 : 16,
        (gui->focused_win == REG_WIN) ? 2 : 3, 0
    );
    WINDOW* reg_inner = newwin(
        16, (gui->focused_win == REG_WIN) ? 40 : 14,
        4, 1
    );
    WINDOW* mem_outer = newwin(
        (gui->focused_win == MEM_WIN) ? 20 : 18, 64,
        (gui->focused_win == MEM_WIN) ? 2 : 3, 16
    );
    WINDOW* mem_inner = newwin(16, 62, 4, 17);
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
