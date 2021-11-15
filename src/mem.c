#include "mem.h"

static MMU* mmu_base;

#define isInData(addr) (addr) < mmu_base->data_len
#define isInStack(addr) (STACK_POINTER - mmu_base->stack_len <= (addr)) && ((addr) < STACK_POINTER)

BYTE mmu_read_instr(ADDR addr) {
    if (addr < mmu_base->instr_len) {
        return mmu_base->instr_mem[addr];
    } else {
        BROADCAST(STAT_MEM_EXCEPTION | ((u64)addr << 32));
        return 0;
    }
}

BYTE mmu_read_data(ADDR addr) {
    if (isInData(addr)) {
        return mmu_base->data_mem[addr];
    } else if (isInStack(addr)) {
        return mmu_base->stack[STACK_POINTER - addr - 1];
    } else {
        BROADCAST(STAT_MEM_EXCEPTION | ((u64)addr << 32));
        return 0;
    }
}

void mmu_write_instr(ADDR addr, BYTE val) {
    if (addr < mmu_base->instr_len) {
        mmu_base->instr_mem[addr] = val;
    } else {
        BROADCAST(STAT_MEM_EXCEPTION | ((u64)addr << 32));
    }
}

void mmu_write_data(ADDR addr, BYTE val) {
    if (isInData(addr)) {
        mmu_base->data_mem[addr] = val;
    } else if (isInStack(addr)) {
        mmu_base->stack[STACK_POINTER - addr - 1] = val;
    } else {
        BROADCAST(STAT_MEM_EXCEPTION | ((u64)addr << 32));
    }
}

void allocate_instr(u64 size) {
    mmu_base->instr_len = size;
    mmu_base->instr_mem = malloc(size * sizeof(BYTE));
}

void allocate_data(u64 size) {
    mmu_base->data_len = size;
    mmu_base->data_mem = malloc(size * sizeof(BYTE));
}

void allocate_stack(u64 size) {
    mmu_base->stack_len = size;
    mmu_base->stack = malloc(size * sizeof(BYTE));
}

void mmu_reset() {
    // keep instr and data, reset stack
    memset(mmu_base->stack, 0, mmu_base->stack_len * sizeof(BYTE));
}

void init_mmu(MMU* mmu) {
    mmu_base = mmu;
    // assign interfaces
    mmu->allocate_instr = allocate_instr;
    mmu->allocate_data = allocate_data;
    mmu->allocate_stack = allocate_stack;
    mmu->read_instr = mmu_read_instr;
    mmu->read_data = mmu_read_data;
    mmu->write_instr = mmu_write_instr;
    mmu->write_data = mmu_write_data;
    mmu->reset = mmu_reset;
}
