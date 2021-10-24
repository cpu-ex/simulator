# decoder

import re

decoder = {
    # tag
    'TAG': re.compile(r'(\S+):'),
    # directive
    'DIREC': re.compile(r'\.(.+)'),
    # pc rd, imm
    'LUI': re.compile(r'(lui)\s+(\S+),\s+(\d+)'),
    'AUIPC': re.compile(r'(auipc)\s+(\S+),\s+(\d+)'),
    # jal rd, tag
    'JAL': re.compile(r'(jal)\s+(\S+),\s+(\S+)'),
    # jalr rd, offset(rs1)
    'JALR': re.compile(r'(jalr)\s+(\S+),\s+(-?\d+)\((\S+)\)'),
    # branch rs1, rs2, tag
    'BEQ': re.compile(r'(beq)\s+(\S+),\s+(\S+),\s+(\S+)'),
    'BNE': re.compile(r'(bne)\s+(\S+),\s+(\S+),\s+(\S+)'),
    'BLT': re.compile(r'(blt)\s+(\S+),\s+(\S+),\s+(\S+)'),
    'BGE': re.compile(r'(bge)\s+(\S+),\s+(\S+),\s+(\S+)'),
    'BLTU': re.compile(r'(bltu)\s+(\S+),\s+(\S+),\s+(\S+)'),
    'BGEU': re.compile(r'(bgeu)\s+(\S+),\s+(\S+),\s+(\S+)'),
    # load rd, offset(rs1)
    'LB': re.compile(r'(lb)\s+(\S+),\s+(-?\d+)\((\S+)\)'),
    'LH': re.compile(r'(lh)\s+(\S+),\s+(-?\d+)\((\S+)\)'),
    'LW': re.compile(r'(lw)\s+(\S+),\s+(-?\d+)\((\S+)\)'),
    'LBU': re.compile(r'(lbu)\s+(\S+),\s+(-?\d+)\((\S+)\)'),
    'LHU': re.compile(r'(lhu)\s+(\S+),\s+(-?\d+)\((\S+)\)'),
    # store rs2, offset(rs1)
    'SB': re.compile(r'(sb)\s+(\S+),\s+(-?\d+)\((\S+)\)'),
    'SH': re.compile(r'(sh)\s+(\S+),\s+(-?\d+)\((\S+)\)'),
    'SW': re.compile(r'(sw)\s+(\S+),\s+(-?\d+)\((\S+)\)'),
    # arith_i rd, rs1, imm
    'ADDI': re.compile(r'(addi)\s+(\S+),\s+(\S+),\s+(-?\d+)'),
    'SLLI': re.compile(r'(slli)\s+(\S+),\s+(\S+),\s+(-?\d+)'),
    'SLTI': re.compile(r'(slti)\s+(\S+),\s+(\S+),\s+(-?\d+)'),
    'SLTIU': re.compile(r'(sltiu)\s+(\S+),\s+(\S+),\s+(-?\d+)'),
    'XORI': re.compile(r'(xori)\s+(\S+),\s+(\S+),\s+(-?\d+)'),
    'SRLI': re.compile(r'(srli)\s+(\S+),\s+(\S+),\s+(-?\d+)'),
    'SRAI': re.compile(r'(srai)\s+(\S+),\s+(\S+),\s+(-?\d+)'),
    'ORI': re.compile(r'(ori)\s+(\S+),\s+(\S+),\s+(-?\d+)'),
    'ANDI': re.compile(r'(andi)\s+(\S+),\s+(\S+),\s+(-?\d+)'),
    # arith rd, rs1, rs2
    'ADD': re.compile(r'(add)\s+(\S+),\s+(\S+),\s+(\S+)'),
    'SUB': re.compile(r'(sub)\s+(\S+),\s+(\S+),\s+(\S+)'),
    'SLL': re.compile(r'(sll)\s+(\S+),\s+(\S+),\s+(\S+)'),
    'SLT': re.compile(r'(slt)\s+(\S+),\s+(\S+),\s+(\S+)'),
    'SLTU': re.compile(r'(sltu)\s+(\S+),\s+(\S+),\s+(\S+)'),
    'XOR': re.compile(r'(xor)\s+(\S+),\s+(\S+),\s+(\S+)'),
    'SRL': re.compile(r'(srl)\s+(\S+),\s+(\S+),\s+(\S+)'),
    'SRA': re.compile(r'(sra)\s+(\S+),\s+(\S+),\s+(\S+)'),
    'OR': re.compile(r'(or)\s+(\S+),\s+(\S+),\s+(\S+)'),
    'AND': re.compile(r'(and)\s+(\S+),\s+(\S+),\s+(\S+)')
}
