#include "core.h"

int main() {
    CORE core;
    init_core(&core);

    /* test code
    * addi a0, zero, 0 : 000000000000 00000 000 01010 0010011
    * addi a1, a0, 4   : 000000000100 01010 000 01011 0010011
    * addi a2, a1, 4   : 000000000100 01011 000 01100 0010011
    * lw t0, 0(a0)     : 000000000000 01010 010 00101 0000011
    * lw t1, 0(a1)     : 000000000000 01011 010 00110 0000011
    * add t2, t0, t1   : 0000000 00110 00101 000 00111 0110011
    * sw t2, 0(a2)     : 0000000 00111 01100 010 00000 0100011
    * simple calculation: 1 + 2 = 3 */
    core.store(0x000, 1, 2);
    core.store(0x004, 2, 2);
    core.store(0x100, 0b00000000000000000000010100010011, 2);
    core.store(0x104, 0b00000000010001010000010110010011, 2);
    core.store(0x108, 0b00000000010001011000011000010011, 2);
    core.store(0x10C, 0b00000000000001010010001010000011, 2);
    core.store(0x110, 0b00000000000001011010001100000011, 2);
    core.store(0x114, 0b00000000011000101000001110110011, 2);
    core.store(0x118, 0b00000000011101100010000000100011, 2);

    for (int i = 0; i < 7; i++)
        core.if_dc_ex();
    
    printf("res = %u\n", core.load(0x008, 2, 0));

    printf("\ndata mem:");
    for (ADDR addr = 0; addr < 0x10; addr++) {
        if (addr % 4 == 0)
            printf("\n%04X: ", addr);
        BYTE val = core.mmu->cache[addr];
        printf("%02X ", val);
    }
    printf("\ncode mem:");
    for (ADDR addr = 0x100; addr < 0x11C; addr++) {
        if (addr % 4 == 0)
            printf("\n%04X: ", addr);
        BYTE val = core.mmu->cache[addr];
        printf("%02X ", val);
    }
    printf("\n");
    return 0;
}
