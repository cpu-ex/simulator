#include "mem.h"

static MMU* mmu_base;

#define STACK_POINTER  0x03FFFFF0
#define UART_IN        0x03FFFFF0
#define UART_IN_VALID  0x03FFFFF4
#define UART_OUT_VALID 0x03FFFFF8
#define UART_OUT       0x03FFFFFC

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
    } else if (addr == UART_IN) {
        return mmu_base->uart_in;
    } else if (addr == UART_IN_VALID) {
        return mmu_base->uart_in_valid;
    } else if (addr == UART_OUT_VALID) {
        return mmu_base->uart_out_valid;
    } else if (addr == UART_OUT) {
        return mmu_base->uart_out;
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
    } else if (addr == UART_IN) {
        mmu_base->uart_in = val;
    } else if (addr == UART_IN_VALID) {
        mmu_base->uart_in_valid = val;
    } else if (addr == UART_OUT_VALID) {
        mmu_base->uart_out_valid = val;
    } else if (addr == UART_OUT) {
        mmu_base->uart_out = val;
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
