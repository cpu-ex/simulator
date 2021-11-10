######################
#        fcmp        #
######################

.globl main

.text

main:

test01:
  li a1, 1
  la a0, test01_data
  flw f0, 0(a0)
  flw f1, 4(a0)
  flw f2, 8(a0)
  lw a3, 12(a0)
  feq a0, f0, f1
  bne a0, a3, fail
  .data
test01_data:
  .float -1.36
  .float -1.36
  .float 0.0
  .word 0x1
  .text

test02:
  li a1, 2
  la a0, test02_data
  flw f0, 0(a0)
  flw f1, 4(a0)
  flw f2, 8(a0)
  lw a3, 12(a0)
  fle a0, f0, f1
  bne a0, a3, fail
  .data
test02_data:
  .float -1.36
  .float -1.36
  .float 0.0
  .word 0x1
  .text

test03:
  li a1, 3
  la a0, test03_data
  flw f0, 0(a0)
  flw f1, 4(a0)
  flw f2, 8(a0)
  lw a3, 12(a0)
  flt a0, f0, f1
  bne a0, a3, fail
  .data
test03_data:
  .float -1.36
  .float -1.36
  .float 0.0
  .word 0x0
  .text

test04:
  li a1, 4
  la a0, test04_data
  flw f0, 0(a0)
  flw f1, 4(a0)
  flw f2, 8(a0)
  lw a3, 12(a0)
  feq a0, f0, f1
  bne a0, a3, fail
  .data
test04_data:
  .float -1.37
  .float -1.36
  .float 0.0
  .word 0x0
  .text

test05:
  li a1, 5
  la a0, test05_data
  flw f0, 0(a0)
  flw f1, 4(a0)
  flw f2, 8(a0)
  lw a3, 12(a0)
  fle a0, f0, f1
  bne a0, a3, fail
  .data
test05_data:
  .float -1.37
  .float -1.36
  .float 0.0
  .word 0x1
  .text

test06:
  li a1, 6
  la a0, test06_data
  flw f0, 0(a0)
  flw f1, 4(a0)
  flw f2, 8(a0)
  lw a3, 12(a0)
  flt a0, f0, f1
  bne a0, a3, fail
  .data
test06_data:
  .float -1.37
  .float -1.36
  .float 0.0
  .word 0x1
  .text

success:
  li a0, 0
  ebreak

fail:
  li a0, 1
  ebreak
