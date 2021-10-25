#include "mem.h"

static MMU* mmu_base;

WORD mmu_read(ADDR addr, int loop) {
    WORD val = 0;
    if (addr == 0x0) {
        // pc final return
    } else if (addr < 0x10000) {
        BROADCAST(STAT_MEM_EXCEPTION | ((u64)addr << 32));
    } else {
        addr -= 0x10000;
        for (int i = loop - 1; i >= 0; i--) {
            val <<= 8;
            if (addr < mmu_base->instr_len) {
                // instruction memory
                val |= mmu_base->instr_cache[addr + i];
            } else if (addr < mmu_base->instr_len + mmu_base->data_len) {
                // data memory
                val |= mmu_base->data_mem[addr + i];
            } else if (STACK_POINTER - mmu_base->stack_len <= addr && addr < STACK_POINTER) {
                // stack
                val |= mmu_base->stack[STACK_POINTER - addr - 4 + i];
            } else {
                // unmapped
            }
        }
    }
    return val;
}

void mmu_write(ADDR addr, WORD val, int loop) {
    if (addr < 0x10000) {
        BROADCAST(STAT_MEM_EXCEPTION | ((u64)addr << 32));
        return;
    } else {
        addr -= 0x10000;
        for (int i = 0; i < loop; i++) {
            if (addr < mmu_base->instr_len) {
                // instruction memory
                mmu_base->instr_cache[addr + i] = val & 0xFF;
            } else if (addr < mmu_base->instr_len + mmu_base->data_len) {
                // data memory
                mmu_base->data_mem[addr + i] = val & 0xFF;
            } else if (STACK_POINTER - mmu_base->stack_len <= addr && addr < STACK_POINTER) {
                // stack
                mmu_base->stack[STACK_POINTER - addr - 4 + i] = val & 0xFF;
            } else {
                // unmapped
            }
            val >>= 8;
        }
    }
}

BYTE read_byte(ADDR addr) { return (BYTE)mmu_read(addr, 1); }
HALF read_half(ADDR addr) { return (HALF)mmu_read(addr, 2); }
WORD read_word(ADDR addr) { return (WORD)mmu_read(addr, 4); }
void write_byte(ADDR addr, BYTE val) { mmu_write(addr, (WORD)val, 1); }
void write_half(ADDR addr, HALF val) { mmu_write(addr, (WORD)val, 2); }
void write_word(ADDR addr, WORD val) { mmu_write(addr, (WORD)val, 4); }

void allocate_instr(u64 size) {
    mmu_base->instr_len = size;
    mmu_base->instr_cache = malloc(size * sizeof(BYTE));
}

void allocate_data(u64 size) {
    mmu_base->data_len = size;
    mmu_base->data_mem = malloc(size * sizeof(BYTE));
}

void allocate_stack(u64 size) {
    mmu_base->stack_len = size;
    mmu_base->stack = malloc(size * sizeof(BYTE));
}

void init_mmu(MMU* mmu) {
    mmu_base = mmu;
    // assign interfaces
    mmu->allocate_instr = allocate_instr;
    mmu->allocate_data = allocate_data;
    mmu->allocate_stack = allocate_stack;
    mmu->read_byte = read_byte;
    mmu->read_half = read_half;
    mmu->read_word = read_word;
    mmu->write_byte = write_byte;
    mmu->write_half = write_half;
    mmu->write_word = write_word;
}
