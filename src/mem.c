#include "mem.h"

#define MAX_ADDR 0x04000000

static MEM* mem_base;

typedef union addr_helper {
    u32 raw;

    struct addr_decoder {
        u16 offset: 8;
        u16 index2: 10;
        u16 index1: 8;
        u16 padding: 6;
    } __attribute__((packed)) d;
} ADDR_HELPER;

void mem_assure_page(ADDR addr) {
    ADDR_HELPER helper = { .raw = addr };
    if (!mem_base->data[helper.d.index1]) {
        mem_base->data[helper.d.index1] = (BYTE**)malloc(0x400 * sizeof(BYTE*));
    }
    if (!mem_base->data[helper.d.index1][helper.d.index2]) {
        mem_base->data[helper.d.index1][helper.d.index2] = (BYTE*)malloc(0x100 * sizeof(BYTE));
    }
}

BYTE mem_read_byte(ADDR addr) {
    ADDR_HELPER helper = { .raw = addr };
    if (!(addr < MAX_ADDR)) {
        BROADCAST(STAT_MEM_EXCEPTION | ((u64)addr << 32));
        return 0;
    } else if (mem_base->data[helper.d.index1] && mem_base->data[helper.d.index1][helper.d.index2]) {
        return mem_base->data[helper.d.index1][helper.d.index2][helper.d.offset];
    } else {
        return 0;
    }
}

void mem_reset_stack(ADDR addr) {
    ADDR_HELPER helper;
    for (; addr < MAX_ADDR; addr++) {
        helper.raw = addr;
        mem_base->data[helper.d.index1][helper.d.index2][helper.d.offset] = 0;
    }
}

void mem_write_byte(ADDR addr, BYTE val) {
    if (!(addr < MAX_ADDR)) {
        BROADCAST(STAT_MEM_EXCEPTION | ((u64)addr << 32));
    } else {
        mem_assure_page(addr);
        ADDR_HELPER helper = { .raw = addr };
        mem_base->data[helper.d.index1][helper.d.index2][helper.d.offset] = val;
    }
}

void init_mem(MEM* mem) {
    mem_base = mem;
    memset(mem->data, 0, 0x100 * sizeof(BYTE**));
    // assign interfaces
    mem->read_byte = mem_read_byte;
    mem->write_byte = mem_write_byte;
    mem->reset_stack = mem_reset_stack;
}
