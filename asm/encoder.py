# encoder

import re

def reg2idx(name: str) -> int:
    reg = {
        'zero': 0,
        'ra': 1,
        'sp': 2,
        'gp': 3,
        'tp': 4,
        't0': 5, 't1': 6, 't2': 7,
        's0': 8, 'fp': 8, 's1': 9,
        'a0': 10, 'a1': 11, 'a2': 12, 'a3': 13, 'a4': 14, 'a5': 15, 'a6': 16, 'a7': 17,
        's2': 18, 's3': 19, 's4': 20, 's5': 21, 's6': 22, 's7': 23, 's8': 24, 's9': 25, 's10': 26, 's11': 27,
        't3': 28, 't4': 29, 't5': 30, 't6': 31
    }
    if (idx := reg.get(name, None)) is not None:
        return idx
    if res := re.match(r'x(\d+)', name):
        idx = int(res.groups()[0])
        if idx < 32:
            return idx
    raise RuntimeError(f'invalid register : {name}')

def tag2offset(name: str, tags: dict, addr: int) -> int:
    if (dst := tags.get(name, None)) is not None:
        return dst - addr
    else:
        raise RuntimeError(f'no tag name : {name}')

# lui imm[31:12] rd[5] 0110111
def lui(instr: tuple, addr: int, tags: dict) -> int:
    rd = reg2idx(instr[1])
    imm = int(instr[2])

    mc = 0b0110111
    mc |= (rd & 0x1F) << 7
    mc |= ((imm >> 12) & 0xFFFFF) << 12
    return mc

# auipc imm[31:12] rd[5] 0010111
def auipc(instr: tuple, addr: int, tags: dict) -> int:
    rd = reg2idx(instr[1])
    imm = int(instr[2])
    
    mc = 0b0010111
    mc |= (rd & 0x1F) << 7
    mc |= ((imm >> 12) & 0xFFFFF) << 12
    return mc

# jal imm[20,10:1,11,19:12] rd 110111
def jal(instr: tuple, addr: int, tags: dict) -> int:
    rd = reg2idx(instr[1])
    imm = tag2offset(instr[2], tags, addr)

    mc = 0b1101111
    mc |= (rd & 0x1F) << 7
    mc |= ((imm & 0x000FF000) >> 12) << 12 # 19:12
    mc |= ((imm & 0x00000800) >> 11) << 20 # 11
    mc |= ((imm & 0x000007FE) >>  1) << 21 # 10:1
    mc |= ((imm & 0x00100000) >> 20) << 31 # 20
    return mc

# jalr imm[11:0] rs1 000 rd 1100111
def jalr(instr: tuple, addr: int, tags: dict) -> int:
    rd = reg2idx(instr[1])
    imm = int(instr[2])
    rs1 = reg2idx(instr[3])

    mc = 0b1100111
    mc |= (rd & 0x1F) << 7
    mc |= 0b000 << 12
    mc |= (rs1 & 0x1F) << 15
    mc |= (imm & 0xFFF) << 20
    return mc

# branch imm[12,10:5] rs2 rs1 funct3 imm[4:1,11] 1100011
def branch(instr: tuple, addr: int, tags: dict) -> int:
    name = instr[0]
    rs1 = reg2idx(instr[1])
    rs2 = reg2idx(instr[2])
    imm = tag2offset(instr[3], tags, addr)

    mc = 0b1100011
    mc |= ((imm & 0x00000800) >> 11) << 7 # 11
    mc |= ((imm & 0x0000001E) >>  1) << 8 # 4:1
    if name == 'BEQ':
        mc |= 0b000 << 12
    elif name == 'BNE':
        mc |= 0b001 << 12
    elif name == 'BLT':
        mc |= 0b100 << 12
    elif name == 'BGE':
        mc |= 0b101 << 12
    elif name == 'BLTU':
        mc |= 0b110 << 12
    elif name == 'BGEU':
        mc |= 0b111 << 12
    else:
        # not suppose to be here
        raise RuntimeError(f'unrecognizable branch type : {name}')
    mc |= (rs1 & 0x1F) << 15
    mc |= (rs2 & 0x1F) << 20
    mc |= ((imm & 0x000007E0) >>  5) << 25 # 10:5
    mc |= ((imm & 0x00001000) >> 12) << 31 # 12
    return mc

# load imm[11:0] rs1 funct3 rd 0000011
def load(instr: tuple, addr: int, tags: dict) -> int:
    name = instr[0]
    rd = reg2idx(instr[1])
    imm = int(instr[2])
    rs1 = reg2idx(instr[3])

    mc = 0b0000011
    mc |= (rd & 0x1F) << 7
    if name == 'LB':
        mc |= 0b000 << 12
    elif name == 'LH':
        mc |= 0b001 << 12
    elif name == 'LW':
        mc |= 0b010 << 12
    elif name == 'LBU':
        mc |= 0b100 << 12
    elif name == 'LHU':
        mc |= 0b101 << 12
    else:
        # not suppose to be here
        raise RuntimeError(f'unrecognizable load type : {name}')
    mc |= (rs1 & 0x1F) << 15
    mc |= (imm & 0xFFF) << 20
    return mc

# store imm[11:5] rs2 rs1 funct3 imm[4:0] 0100011
def store(instr: tuple, addr: int, tags: dict) -> int:
    name = instr[0]
    rs2 = reg2idx(instr[1])
    imm = int(instr[2])
    rs1 = reg2idx(instr[3])

    mc = 0b0100011
    mc |= (imm & 0xF) << 7 # [4:0]
    if name == 'SB':
        mc |= 0b000 << 12
    elif name == 'SH':
        mc |= 0b001 << 12
    elif name == 'SW':
        mc |= 0b010 << 12
    else:
        # not suppose to be here
        raise RuntimeError(f'unrecognizable store type : {name}')
    mc |= (rs1 & 0x1F) << 15
    mc |= (rs2 & 0x1F) << 20
    mc |= ((imm & 0xFE0) >> 5) << 25
    return mc

# arith_i imm[11:0] rs1 funct3 rd 0010011
def arith_i(instr: tuple, addr: int, tags: dict) -> int:
    name = instr[0]
    rd = reg2idx(instr[1])
    rs1 = reg2idx(instr[2])
    imm = int(instr[3])

    mc = 0b0010011
    mc |= (rd & 0x1F) << 7
    mc |= (rs1 & 0x1F) << 15
    if name == 'ADDI':
        mc |= 0b000 << 12
        mc |= (imm & 0xFFF) << 20
    elif name == 'SLTI':
        mc |= 0b010 << 12
        mc |= (imm & 0xFFF) << 20
    elif name == 'SLTIU':
        mc |= 0b011 << 12
        mc |= (imm & 0xFFF) << 20
    elif name == 'XORI':
        mc |= 0b100 << 12
        mc |= (imm & 0xFFF) << 20
    elif name == 'ORI':
        mc |= 0b110 << 12
        mc |= (imm & 0xFFF) << 20
    elif name == 'ANDI':
        mc |= 0b111 << 12
        mc |= (imm & 0xFFF) << 20
    elif name == 'SLLI':
        mc |= 0b001 << 12
        mc |= (imm & 0x1F) << 20 # shamt
        mc |= 0b0000000 << 25
    elif name == 'SRLI':
        mc |= 0b101 << 12
        mc |= (imm & 0x1F) << 20 # shamt
        mc |= 0b0000000 << 25
    elif name == 'SRAI':
        mc |= 0b101 << 12
        mc |= (imm & 0x1F) << 20 # shamt
        mc |= 0b0100000 << 25
    else:
        # not suppose to be here
        raise RuntimeError(f'unrecognizable arith-i type : {name}')
    return mc

# arith funct7 rs2 rs1 funct3 rd 0110011
def arith(instr: tuple, addr: int, tags: dict) -> int:
    name = instr[0]
    rd = reg2idx(instr[1])
    rs1 = reg2idx(instr[2])
    rs2 = reg2idx(instr[3])

    mc = 0b0110011
    mc |= (rd & 0x1F) << 7
    mc |= (rs1 & 0x1F) << 15
    mc |= (rs2 & 0x1F) << 20
    if name == 'ADD':
        mc |= 0b000 << 12
        mc |= 0b0000000 << 25
    elif name == 'SUB':
        mc |= 0b000 << 12
        mc |= 0b0100000 << 25
    elif name == 'SLL':
        mc |= 0b001 << 12
        mc |= 0b0000000 << 25
    elif name == 'SLT':
        mc |= 0b010 << 12
        mc |= 0b0000000 << 25
    elif name == 'SLTU':
        mc |= 0b011 << 12
        mc |= 0b0000000 << 25
    elif name == 'XOR':
        mc |= 0b100 << 12
        mc |= 0b0000000 << 25
    elif name == 'SRL':
        mc |= 0b101 << 12
        mc |= 0b0000000 << 25
    elif name == 'SRA':
        mc |= 0b101 << 12
        mc |= 0b0100000 << 25
    elif name == 'OR':
        mc |= 0b110 << 12
        mc |= 0b0000000 << 25
    elif name == 'AND':
        mc |= 0b111 << 12
        mc |= 0b0000000 << 25
    else:
        # not suppose to be here
        raise RuntimeError(f'unrecognizable arith type : {name}')
    return mc

encoder = {
    # pc
    'LUI': lui,
    'AUIPC': auipc,
    # jal
    'JAL': jal,
    # jalr
    'JALR': jalr,
    # branch
    'BEQ': branch,
    'BNE': branch,
    'BLT': branch,
    'BGE': branch,
    'BLTU': branch,
    'BGEU': branch,
    # load
    'LB': load,
    'LH': load,
    'LW': load,
    'LBU': load,
    'LHU': load,
    # store
    'SB': store,
    'SH': store,
    'SW': store,
    # arith_i
    'ADDI': arith_i,
    'SLLI': arith_i,
    'SLTI': arith_i,
    'SLTIU': arith_i,
    'XORI': arith_i,
    'SRLI': arith_i,
    'SRAI': arith_i,
    'ORI': arith_i,
    'ANDI': arith_i,
    # arith
    'ADD': arith,
    'SUB': arith,
    'SLL': arith,
    'SLT': arith,
    'SLTU': arith,
    'XOR': arith,
    'SRL': arith,
    'SRA': arith,
    'OR': arith,
    'AND': arith
}
