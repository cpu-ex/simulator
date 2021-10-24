    .text
    .align 2
    .globl main
main:
    auipc a0, 0
    ori a0, a0, 256
    addi a1, a0, 4
    addi a2, a1, 4
    lw t0, 0(a0)
    lw t1, 0(a1)
    add t2, t0, t1
    sw t2, 0(a2)
    jalr zero, 0(ra)
