  .globl main
  .text
main:
  li t2, 0x03FFFFF0
  addi t0, zero, 256
loop: # wait fifo reset
  addi t0, t0, -1
  blt zero, t0, loop
out99:
  lw t0, 8(t2) # uart_out_valid
  beq t0, zero, out99
  addi t0, zero, 0x99
  sw t0, 12(t2) # uart_tx
# receive program size  
load1:
  lw t0, 4(t2) # uart_in_valid
  beq t0, zero, load1
  lw t1, 0(t2) # uart_rx
  slli t1, t1, 8
load2:
  lw t0, 4(t2) # uart_in_valid
  beq t0, zero, load2
  lw t0, 0(t2) # uart_rx
  or t1, t1, t0
  slli t1, t1, 8
load3:
  lw t0, 4(t2) # uart_in_valid
  beq t0, zero, load3
  lw t0, 0(t2) # uart_rx
  or t1, t1, t0
  slli t1, t1, 8
load4:
  lw t0, 4(t2) # uart_in_valid
  beq t0, zero, load4
  lw t0, 0(t2) # uart_rx
  or t1, t1, t0

# receive program   
  li a2, 0x100 # program start point
pload1:
  lw a0, 4(t2) # uart_in_valid
  beq a0, zero, pload1
  lw a1, 0(t2) # uart_rx
  slli a1, a1, 8
pload2:
  lw a0, 4(t2) # uart_in_valid
  beq a0, zero, pload2
  lw a0, 0(t2) # uart_rx
  or a1, a1, a0
  slli a1, a1, 8
pload3:
  lw a0, 4(t2) # uart_in_valid
  beq a0, zero, pload3
  lw a0, 0(t2) # uart_rx
  or a1, a1, a0
  slli a1, a1, 8
pload4:
  lw a0, 4(t2) # uart_in_valid
  beq a0, zero, pload4
  lw a0, 0(t2) # uart_rx
  or a1, a1, a0

  swi a1, 0(a2)
  addi a2, a2, 4
  addi t1, t1, -4
  blt zero, t1, pload1

# outaa:
#   lw t0, 8(t2) # uart_out_valid
#   beq t0, zero, outaa
#   addi t0, zero, 0xaa
#   sw t0, 12(t2) # uart_tx

  # jump progarm
  ecall
