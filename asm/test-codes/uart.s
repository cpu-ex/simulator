    .globl main
    .text
main:
    li t0, 0x3FFFFFC
    li a1, 10
for:
    bge a0, a1, endfor
    sw a0, 0(t0)
    addi a0, a0, 1
    j for
endfor:
    ebreak
