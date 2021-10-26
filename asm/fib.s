    .text
    .align 2
    .globl main
main:
    addi a0, a0, 4
fib: # a0 = n
    addi sp, sp, -12
    sw ra, 0(sp)
    addi t0, zero, 3
    blt a0, t0, tb

    sw a0, 4(sp) # save current n
    addi a0, a0, -1 # a0 = n - 1
    jal ra, fib # fib (n - 1)
    sw a0, 8(sp) # save fib (n - 1)
    
    lw a0, 4(sp)
    addi a0, a0, -2 # a0 = n - 2
    jal ra, fib
    lw a1, 8(sp)
    add a0, a0, a1
    jal zero, final
tb:
    addi a0, zero, 1
final:
    lw ra, 0(sp)
    addi sp, sp, 12
    jalr zero, 0(ra)
