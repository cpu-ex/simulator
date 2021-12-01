#include "mem.h"

typedef union mem_addr_helper {
    u32 raw;

    struct addr_decoder {
        u16 offset: 8;
        u16 index2: 10;
        u16 index1: 8;
        u16 padding: 6;
    } __attribute__((packed)) d;
} MEM_ADDR_HELPER;

void mem_assure_page(MEM* mem, ADDR addr) {
    MEM_ADDR_HELPER helper = { .raw = addr };
    if (!mem->data[helper.d.index1]) {
        mem->data[helper.d.index1] = (BYTE**)malloc(0x400 * sizeof(BYTE*));
        memset(mem->data[helper.d.index1], 0, 0x400 * sizeof(BYTE*));
    }
    if (!mem->data[helper.d.index1][helper.d.index2]) {
        mem->data[helper.d.index1][helper.d.index2] = (BYTE*)malloc(0x100 * sizeof(BYTE));
        memset(mem->data[helper.d.index1][helper.d.index2], 0, 0x100 * sizeof(BYTE));
    }
}

BYTE mem_read_byte(MEM* mem, ADDR addr) {
    MEM_ADDR_HELPER helper = { .raw = addr };
    if (!(addr < MAX_ADDR)) {
        BROADCAST(STAT_MEM_EXCEPTION | ((u64)addr << 32));
        return 0;
    } else if (mem->data[helper.d.index1] && mem->data[helper.d.index1][helper.d.index2]) {
        return mem->data[helper.d.index1][helper.d.index2][helper.d.offset];
    } else {
        return 0;
    }
}

void mem_write_byte(MEM* mem, ADDR addr, BYTE val) {
    if (!(addr < MAX_ADDR)) {
        BROADCAST(STAT_MEM_EXCEPTION | ((u64)addr << 32));
    } else {
        mem_assure_page(mem, addr);
        MEM_ADDR_HELPER helper = { .raw = addr };
        mem->data[helper.d.index1][helper.d.index2][helper.d.offset] = val;
    }
}

void mem_reset_stack(MEM* mem, ADDR addr) {
    for (; addr < MAX_ADDR; addr++)
        mem->write_byte(mem, addr, 0);
}

void init_mem(MEM* mem) {
    memset(mem->data, 0, 0x100 * sizeof(BYTE**));
    // assign interfaces
    mem->read_byte = mem_read_byte;
    mem->write_byte = mem_write_byte;
    mem->reset_stack = mem_reset_stack;
}
