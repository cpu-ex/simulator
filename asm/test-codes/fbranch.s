######################
#      f-branch      #
######################

.globl main

.text

main:

test01:
  li a1, 1
  la a0, test01_data
  flw f0, 0(a0)
  flw f1, 4(a0)
  bfeq f0, f1, test02
  j fail
  .data
test01_data:
  .float -1.36
  .float -1.36
  .text

test02:
  li a1, 2
  la a0, test02_data
  flw f0, 0(a0)
  flw f1, 4(a0)
  bfle f0, f1, test03
  j fail
  .data
test02_data:
  .float -1.36
  .float -1.36
  .text

test03:
  li a1, 3
  la a0, test03_data
  flw f0, 0(a0)
  flw f1, 4(a0)
  bfeq f0, f1, fail
  .data
test03_data:
  .float -1.37
  .float -1.36
  .text

test04:
  li a1, 4
  la a0, test04_data
  flw f0, 0(a0)
  flw f1, 4(a0)
  bfle f0, f1, fail
  .data
test04_data:
  .float -1.36
  .float -1.37
  .text

success:
  li a0, 0
  ebreak

fail:
  li a0, 1
  ebreak
