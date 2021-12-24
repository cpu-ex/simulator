    .globl main

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

read_byte:
    li t0, 0x3FFFFFC
    lw a0, 0(t0)
    ret

# sum(0 .. 9) = 45
main:
    addi s0, zero, 0
    addi s1, zero, 10
    addi a1, zero, 0
for:
    bge s0, s1, endfor
    call read_byte
    add a1, a1, a0
    addi s0, s0, 1
    j for
endfor:
    mv a0, a1
    call print_int
    ebreak
