#include <stdlib.h>
#include "mem.h"

static MMU* mmu_base;

WORD mmu_read(ADDR addr, int loop) {
    WORD val = 0;
    for (int i = loop - 1; i >= 0; i--) {
        val <<= 8;
        val |= mmu_base->cache[addr + i];
    }
    return val;
}

void mmu_write(ADDR addr, WORD val, int loop) {
    for (int i = 0; i < loop; i++) {
        mmu_base->cache[addr + i] = val & 0xFF;
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

    mmu->cache = malloc(CACHE_LEN * sizeof(BYTE));
    mmu->read_byte = read_byte;
    mmu->read_half = read_half;
    mmu->read_word = read_word;
    mmu->write_byte = write_byte;
    mmu->write_half = write_half;
    mmu->write_word = write_word;
}
