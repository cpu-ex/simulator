######################
#     jal TESTS      #
######################

.globl main

.text

main:

test01:
  li a1, 1
  li ra, 0
  jal x4, target_2
linkaddr_2:
  nop
  nop
  j fail
target_2:
  la  x2, linkaddr_2
  bne x2, x4, fail

success:
  li a0, 0
  ebreak

fail:
  li a0, 1
  ebreak
