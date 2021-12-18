  .globl main
  .text
main:
  addi t0, zero, 256
loop: # wait fifo reset
  addi t0, t0, -1
  blt zero, t0, loop

  addi t0, zero, 0x99
  sw t0, 0(zero) # uart_tx
# receive program size  
  lw t1, 0(zero) # uart_rx
  slli t1, t1, 8

  lw t0, 0(zero) # uart_rx
  or t1, t1, t0
  slli t1, t1, 8

  lw t0, 0(zero) # uart_rx
  or t1, t1, t0
  slli t1, t1, 8

  lw t0, 0(zero) # uart_rx
  or t1, t1, t0

# receive program   
  li a2, 0x100 # program start point
pload:

  lw a1, 0(zero) # uart_rx
  slli a1, a1, 8

  lw a0, 0(zero) # uart_rx
  or a1, a1, a0
  slli a1, a1, 8

  lw a0, 0(zero) # uart_rx
  or a1, a1, a0
  slli a1, a1, 8

  lw a0, 0(zero) # uart_rx
  or a1, a1, a0

  swi a1, 0(a2)
  addi a2, a2, 4
  addi t1, t1, -4
  blt zero, t1, pload

  addi t0, zero, 0xaa
  sw t0, 0(zero) # uart_tx

  # jump progarm
  ecall
