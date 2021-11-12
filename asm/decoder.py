# decoder

import re

decoder = {
    # tag
    'TAG': (re.compile(r'(\S+):'), 1),
    # directive
    'DIREC-DATA': (re.compile(r'\.(\S+?)\s+(.+)'), 1),
    'DIREC-PLAIN': (re.compile(r'\.(\S+)'), 1),


    # RV32I
    # pc rd, imm
    'LUI': (re.compile(r'(lui)\s+(\S+),\s*(-?\S+)'), 1),
    'AUIPC': (re.compile(r'(auipc)\s+(\S+),\s*(-?\S+)'), 1),
    # jal rd, tag
    'JAL': (re.compile(r'(jal)\s+(\S+),\s*(\S+)'), 1),
    # jalr rd, offset(rs1)
    'JALR': (re.compile(r'(jalr)\s+(\S+),\s*(-?\S+)\((\S+)\)'), 1),
    # branch rs1, rs2, tag
    'BEQ': (re.compile(r'(beq)\s+(\S+),\s*(\S+),\s*(\S+)'), 1),
    'BNE': (re.compile(r'(bne)\s+(\S+),\s*(\S+),\s*(\S+)'), 1),
    'BLT': (re.compile(r'(blt)\s+(\S+),\s*(\S+),\s*(\S+)'), 1),
    'BGE': (re.compile(r'(bge)\s+(\S+),\s*(\S+),\s*(\S+)'), 1),
    'BLTU': (re.compile(r'(bltu)\s+(\S+),\s*(\S+),\s*(\S+)'), 1),
    'BGEU': (re.compile(r'(bgeu)\s+(\S+),\s*(\S+),\s*(\S+)'), 1),
    # load rd, offset(rs1)
    'LB': (re.compile(r'(lb)\s+(\S+),\s*(-?\S+)\((\S+)\)'), 1),
    'LH': (re.compile(r'(lh)\s+(\S+),\s*(-?\S+)\((\S+)\)'), 1),
    'LW': (re.compile(r'(lw)\s+(\S+),\s*(-?\S+)\((\S+)\)'), 1),
    'LBU': (re.compile(r'(lbu)\s+(\S+),\s*(-?\S+)\((\S+)\)'), 1),
    'LHU': (re.compile(r'(lhu)\s+(\S+),\s*(-?\S+)\((\S+)\)'), 1),
    # store rs2, offset(rs1)
    'SB': (re.compile(r'(sb)\s+(\S+),\s*(-?\S+)\((\S+)\)'), 1),
    'SH': (re.compile(r'(sh)\s+(\S+),\s*(-?\S+)\((\S+)\)'), 1),
    'SW': (re.compile(r'(sw)\s+(\S+),\s*(-?\S+)\((\S+)\)'), 1),
    # arith_i rd, rs1, imm
    'ADDI': (re.compile(r'(addi)\s+(\S+),\s*(\S+),\s*(-?\S+)'), 1),
    'SLLI': (re.compile(r'(slli)\s+(\S+),\s*(\S+),\s*(-?\S+)'), 1),
    'SLTI': (re.compile(r'(slti)\s+(\S+),\s*(\S+),\s*(-?\S+)'), 1),
    'SLTIU': (re.compile(r'(sltiu)\s+(\S+),\s*(\S+),\s*(-?\S+)'), 1),
    'XORI': (re.compile(r'(xori)\s+(\S+),\s*(\S+),\s*(-?\S+)'), 1),
    'SRLI': (re.compile(r'(srli)\s+(\S+),\s*(\S+),\s*(-?\S+)'), 1),
    'SRAI': (re.compile(r'(srai)\s+(\S+),\s*(\S+),\s*(-?\S+)'), 1),
    'ORI': (re.compile(r'(ori)\s+(\S+),\s*(\S+),\s*(-?\S+)'), 1),
    'ANDI': (re.compile(r'(andi)\s+(\S+),\s*(\S+),\s*(-?\S+)'), 1),
    # arith rd, rs1, rs2
    'ADD': (re.compile(r'(add)\s+(\S+),\s*(\S+),\s*(\S+)'), 1),
    'SUB': (re.compile(r'(sub)\s+(\S+),\s*(\S+),\s*(\S+)'), 1),
    'SLL': (re.compile(r'(sll)\s+(\S+),\s*(\S+),\s*(\S+)'), 1),
    'SLT': (re.compile(r'(slt)\s+(\S+),\s*(\S+),\s*(\S+)'), 1),
    'SLTU': (re.compile(r'(sltu)\s+(\S+),\s*(\S+),\s*(\S+)'), 1),
    'XOR': (re.compile(r'(xor)\s+(\S+),\s*(\S+),\s*(\S+)'), 1),
    'SRL': (re.compile(r'(srl)\s+(\S+),\s*(\S+),\s*(\S+)'), 1),
    'SRA': (re.compile(r'(sra)\s+(\S+),\s*(\S+),\s*(\S+)'), 1),
    'OR': (re.compile(r'(or)\s+(\S+),\s*(\S+),\s*(\S+)'), 1),
    'AND': (re.compile(r'(and)\s+(\S+),\s*(\S+),\s*(\S+)'), 1),
    # env
    'EBREAK': None,
    # pseudo
    'PSEUDO-NOP': (re.compile(r'(nop)'), 1),
    'PSEUDO-LI': (re.compile(r'(li)\s+(\S+),\s*(-?\S+)'), 2),
    'PSEUDO-LA': (re.compile(r'(la)\s+(\S+),\s*(\S+)'), 2),
    'PSEUDO-NOT': (re.compile(r'(not)\s+(\S+),\s*(\S+)'), 1),
    'PSEUDO-MV': (re.compile(r'(mv)\s+(\S+),\s*(\S+)'), 1),
    'PSEUDO-J': (re.compile(r'(j)\s+(\S+)'), 1),
    'PSEUDO-JAL': (re.compile(r'(jal)\s+(\S+)'), 1),
    'PSEUDO-JALR': (re.compile(r'(jalr)\s+(\S+)'), 1),
    'PSEUDO-RET': (re.compile(r'(ret)'), 1),


    # RV32M
    'MUL': (re.compile(r'(mul)\s+(\S+),\s*(\S+),\s*(\S+)'), 1),
    'DIV': (re.compile(r'(div)\s+(\S+),\s*(\S+),\s*(\S+)'), 1),
    'DIVU': (re.compile(r'(divu)\s+(\S+),\s*(\S+),\s*(\S+)'), 1),
    'REM': (re.compile(r'(rem)\s+(\S+),\s*(\S+),\s*(\S+)'), 1),
    'REMU': (re.compile(r'(remu)\s+(\S+),\s*(\S+),\s*(\S+)'), 1),


    # RV32F
    'FLW': (re.compile(r'(flw)\s+(\S+),\s*(-?\S+)\((\S+)\)'), 1),
    'FSW': (re.compile(r'(fsw)\s+(\S+),\s*(-?\S+)\((\S+)\)'), 1),
    'FMVXW': (re.compile(r'(fmv\.x\.w)\s+(\S+),\s*(\S+)'), 1),
    'FMVWX': (re.compile(r'(fmv\.w\.x)\s+(\S+),\s*(\S+)'), 1),
    'FADD': (re.compile(r'(fadd)\s+(\S+),\s*(\S+),\s*(\S+)'), 1),
    'FSUB': (re.compile(r'(fsub)\s+(\S+),\s*(\S+),\s*(\S+)'), 1),
    'FMUL': (re.compile(r'(fmul)\s+(\S+),\s*(\S+),\s*(\S+)'), 1),
    'FDIV': (re.compile(r'(fdiv)\s+(\S+),\s*(\S+),\s*(\S+)'), 1),
    'FSQRT': (re.compile(r'(fsqrt)\s+(\S+),\s*(\S+)'), 1),
    'FEQ': (re.compile(r'(feq)\s+(\S+),\s*(\S+),\s*(\S+)'), 1),
    'FLT': (re.compile(r'(flt)\s+(\S+),\s*(\S+),\s*(\S+)'), 1),
    'FLE': (re.compile(r'(fle)\s+(\S+),\s*(\S+),\s*(\S+)'), 1),
    'FCVTSW': (re.compile(r'(fcvt\.s\.w)\s+(\S+),\s*(\S+)'), 1),
    'FCVTWS': (re.compile(r'(fcvt\.w\.s)\s+(\S+),\s*(\S+)'), 1),
    'FCVTSWU': (re.compile(r'(fcvt\.s\.wu)\s+(\S+),\s*(\S+)'), 1),
    'FCVTWUS': (re.compile(r'(fcvt\.wu\.s)\s+(\S+),\s*(\S+)'), 1),
    'FSGNJ': (re.compile(r'(fsgnj)\s+(\S+),\s*(\S+),\s*(\S+)'), 1),
    'FSGNJN': (re.compile(r'(fsgnjn)\s+(\S+),\s*(\S+),\s*(\S+)'), 1),
    'FSGNJX': (re.compile(r'(fsgnjx)\s+(\S+),\s*(\S+),\s*(\S+)'), 1),

    # special
    'SWI': (re.compile(r'(swi)\s+(\S+),\s*(-?\S+)\((\S+)\)'), 1),
}

# ebreak for sim
ebreak_sim = (re.compile(r'(ebreak)'), 1)
# ebreak for fpga
ebreak_fpga = (re.compile(r'(ebreak)'), 1)

def getDecoder(forSim: bool) -> dict:
    decoder['EBREAK'] = ebreak_sim if forSim else ebreak_fpga
    return decoder
