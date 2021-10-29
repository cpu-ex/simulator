    .globl main

    .data
const:
    .byte 10

    .text
fib: # a0 = n
    addi sp, sp, -12
    sw ra, 0(sp)
    addi t0, zero, 3
    blt a0, t0, tb

    sw a0, 4(sp) # save current n
    addi a0, a0, -1 # a0 = n - 1
    jal fib # fib (n - 1)
    sw a0, 8(sp) # save fib (n - 1)
    
    lw a0, 4(sp)
    addi a0, a0, -2 # a0 = n - 2
    jal fib # fib (n - 2)
    lw a1, 8(sp)
    add a0, a0, a1
    j final
tb:
    addi a0, zero, 1
final:
    lw ra, 0(sp)
    addi sp, sp, 12
    ret

main:
    la t0, const
    lb a0, 0(t0)
    # or remove the data section
    # and use this instead: li a0, 10
    addi sp, sp, -4
    sw ra, 0(sp)
    jal fib # call fib(a0)
    lw ra, 0(sp)
    addi sp, sp, 4
    ret
