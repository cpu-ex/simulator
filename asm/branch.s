    .text
    .align 2
    .globl main
main:
    addi a0, zero, 1
    addi a1, zero, 3
    blt a1, a2, tb
    addi a2, a0, 0
    jal x0, final
tb:
    addi a2, a1, 0
final:
    jalr zero, 0(ra)
