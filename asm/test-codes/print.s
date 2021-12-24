	.globl main

# def dec2char(a0):
# 	a2, a0, a1 = a0, 0, 0
# 	while a2 >= 100:
# 		a2 -= 100
# 		a0 += 1
# 	while a2 >= 10:
# 		a2 -= 10
# 		a1 += 1
# 	return a0, a1, a2

	.text
print_int:
	mv a2, a0
	addi a0, zero, 0
	addi a1, zero, 0
	addi t1, zero, 100
	addi t2, zero, 10
hundreds:
	blt a2, t1, tens
	sub a2, a2, t1
	addi a0, a0, 1
	j hundreds
tens:
	blt a2, t2, ones
	sub a2, a2, t2
	addi a1, a1, 1
	j tens
ones: # a2 is digit of ones
	li t0, 0x3FFFFFC
	addi a0, a0, 48
	sw a0, 0(t0)
	addi a1, a1, 48
	sw a1, 0(t0)
	addi a2, a2, 48
	sw a2, 0(t0)
	ret

print_char:
	li t0, 0x3FFFFFC
	sw a0, 0(t0)
	ret

main:
	addi a0, zero, 123
	call print_int
	addi a0, zero, 61
	call print_char
	addi a0, zero, 49
	call print_char
	addi a0, zero, 50
	call print_char
	addi a0, zero, 51
	call print_char
	ebreak
