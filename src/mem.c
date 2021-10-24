#include <stdlib.h>
#include "mem.h"

static MMU* mmu_base;
static u32 INSTR_LEN, DATA_LEN, STACK_LEN;

WORD mmu_read(ADDR addr, int loop) {
    addr -= 0x10000;
    WORD val = 0;
    for (int i = loop - 1; i >= 0; i--) {
        val <<= 8;
        if (addr < INSTR_LEN) {
            // instruction memory
            val |= mmu_base->instr_cache[addr + i];
        } else if (addr < INSTR_LEN + DATA_LEN) {
            // data memory
            val |= mmu_base->data_mem[addr + i];
        } else if (addr >= STACK_POINTER - STACK_LEN) {
            // stack
            val |= mmu_base->stack[STACK_POINTER - addr - 4 + i];
        } else {
            // unmapped
        }
    }
    return val;
}

void mmu_write(ADDR addr, WORD val, int loop) {
    addr -= 0x10000;
    for (int i = 0; i < loop; i++) {
        if (addr < INSTR_LEN) {
            // instruction memory
            mmu_base->instr_cache[addr + i] = val & 0xFF;
        } else if (addr < INSTR_LEN + DATA_LEN) {
            // data memory
            mmu_base->data_mem[addr + i] = val & 0xFF;
        } else if (addr >= STACK_POINTER - STACK_LEN) {
            // stack
            mmu_base->stack[STACK_POINTER - addr - 4 + i] = val & 0xFF;
        } else {
            // unmapped
        }
        val >>= 8;
    }
}

BYTE read_byte(ADDR addr) { return (BYTE)mmu_read(addr, 1); }
HALF read_half(ADDR addr) { return (HALF)mmu_read(addr, 2); }
WORD read_word(ADDR addr) { return (WORD)mmu_read(addr, 4); }
void write_byte(ADDR addr, BYTE val) { mmu_write(addr, (WORD)val, 1); }
void write_half(ADDR addr, HALF val) { mmu_write(addr, (WORD)val, 2); }
void write_word(ADDR addr, WORD val) { mmu_write(addr, (WORD)val, 4); }

void init_mmu(MMU* mmu) {
    mmu_base = mmu;
    // allocate memories
    INSTR_LEN = 0x100;
    DATA_LEN = 0x100;
    STACK_LEN = 0x100;
    mmu->instr_cache = malloc(INSTR_LEN * sizeof(BYTE));
    mmu->data_mem = malloc(DATA_LEN * sizeof(BYTE));
    mmu->stack = malloc(STACK_LEN * sizeof(BYTE));
    // assign interfaces
    mmu->read_byte = read_byte;
    mmu->read_half = read_half;
    mmu->read_word = read_word;
    mmu->write_byte = write_byte;
    mmu->write_half = write_half;
    mmu->write_word = write_word;
}
