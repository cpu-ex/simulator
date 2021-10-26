#include "mem.h"

static MMU* mmu_base;

#define isInInstr(addr) ((addr) - 0x10000) < mmu_base->instr_len
#define isInData(addr) ((addr) - 0x10000) < (mmu_base->instr_len + mmu_base->data_len)
#define isInStack(addr) (STACK_POINTER - mmu_base->stack_len <= (addr)) && ((addr) < STACK_POINTER)
#define map2instr(addr) (addr) - 0x10000
#define map2data(addr) (addr) - 0x10000
#define map2stack(addr) mmu_base->stack_len - (STACK_POINTER - (addr))

WORD mmu_read(ADDR addr, int loop) {
    WORD val = 0;
    if (addr == 0x0) {
        // pc final return
    } else if (addr < 0x10000) {
        BROADCAST(STAT_MEM_EXCEPTION | ((u64)addr << 32));
    } else {
        for (int i = loop - 1; i >= 0; i--) {
            val <<= 8;
            if (isInInstr(addr)) {
                // instruction memory
                val |= mmu_base->instr_cache[map2instr(addr) + i];
            } else if (isInData(addr)) {
                // data memory
                val |= mmu_base->data_mem[map2data(addr) + i];
            } else if (isInStack(addr)) {
                // stack
                val |= mmu_base->stack[map2stack(addr) + i];
            } else {
                // unmapped
                BROADCAST(STAT_MEM_EXCEPTION | ((u64)addr << 32));
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
        for (int i = 0; i < loop; i++) {
            if (isInInstr(addr)) {
                // instruction memory
                mmu_base->instr_cache[map2instr(addr) + i] = val & 0xFF;
            } else if (isInData(addr)) {
                // data memory
                mmu_base->data_mem[map2data(addr) + i] = val & 0xFF;
            } else if (isInStack(addr)) {
                // stack
                mmu_base->stack[map2stack(addr) + i] = val & 0xFF;
            } else {
                // unmapped
                BROADCAST(STAT_MEM_EXCEPTION | ((u64)addr << 32));
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
