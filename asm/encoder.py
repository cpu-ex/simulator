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
    elif res := re.match(r'x(\d+)', name):
        idx = res.groups()[0]
        if idx < 32:
            return idx
        else:
            pass #TODO: raise exception
    else:
        pass #TODO: raise exception

def tag2offset(name: str, tags: dict, addr: int) -> int:
    if (dst := tags.get(name, None)) is not None:
        return dst - addr
    else:
        pass #TODO: raise exception

# lui imm[31:12] rd[5] 0110111
def lui(instr: tuple, addr: int, tags: dict) -> str:
    rd = reg2idx(instr[1])
    imm = int(instr[2])

    mc = 0b0110111
    mc |= (rd & 0x1F) << 7
    mc |= ((imm >> 12) & 0xFFFFF) << 12
    return bin(mc)[2:].zfill(32)

# auipc imm[31:12] rd[5] 0010111
def auipc(instr: tuple, addr: int, tags: dict) -> str:
    rd = reg2idx(instr[1])
    imm = int(instr[2])
    
    mc = 0b0010111
    mc |= (rd & 0x1F) << 7
    mc |= ((imm >> 12) & 0xFFFFF) << 12
    return bin(mc)[2:].zfill(32)

# jal imm[20,10:1,11,19:12] rd 110111
def jal(instr: tuple, addr: int, tags: dict) -> str:
    rd = reg2idx(instr[1])
    imm = tag2offset(int(instr[2]), tags, addr)

    mc = 0b110111
    mc |= (rd & 0x1F) << 7
    mc |= ((imm & 0x000FF000) >> 12) << 12 # 19:12
    mc |= ((imm & 0x00000800) >> 11) << 20 # 11
    mc |= ((imm & 0x000007FE) >>  1) << 21 # 10:1
    mc |= ((imm & 0x00100000) >> 20) << 31 # 20
    return bin(mc)[2:].zfill(32)

# jalr imm[11:0] rs1 000 rd 1100111
def jalr(instr: tuple, addr: int, tags: dict) -> str:
    rd = reg2idx(instr[1])
    imm = int(instr[2])
    rs1 = reg2idx(instr[3])

    mc = 0b1100111
    mc |= (rd & 0x1F) << 7
    mc |= 0b000 << 12
    mc |= (rs1 & 0x1F) << 15
    mc |= (imm & 0xFFF) << 20
    return bin(mc)[2:].zfill(32)

# branch imm[12,10:5] rs2 rs1 funct3 imm[4:1,11] 1100011
def branch(instr: tuple, addr: int, tags: dict) -> str:
    name = instr[0]
    rs1 = reg2idx(instr[1])
    rs2 = reg2idx(instr[2])
    imm = tag2offset(int(instr[3]), tags, addr)

    mc = 1100011
    mc |= ((imm & 0x00000800) >> 11) << 7 # 11
    mc |= ((imm & 0x0000001E) >>  1) << 8 # 4:1
    if name == 'beq':
        mc |= 0b000 << 12
    elif name == 'bne':
        mc |= 0b001 << 12
    elif name == 'blt':
        mc |= 0b100 << 12
    elif name == 'bge':
        mc |= 0b101 << 12
    elif name == 'bltu':
        mc |= 0b110 << 12
    elif name == 'bgeu':
        mc |= 0b111 << 12
    mc |= (rs1 & 0x1F) << 15
    mc |= (rs2 & 0x1F) << 20
    mc |= ((imm & 0x000007E0) >>  5) << 25 # 10:5
    mc |= ((imm & 0x00001000) >> 12) << 31 # 12
    return bin(mc)[2:].zfill(32)

# load imm[11:0] rs1 funct3 rd 0000011
def load(instr: tuple, addr: int, tags: dict) -> str:
    name = instr[0]
    rd = reg2idx(instr[1])
    imm = int(instr[2])
    rs1 = reg2idx(instr[3])

    mc = 0b0000011
    mc |= (rd & 0x1F) << 7
    if name == 'lb':
        mc |= 0b000 << 12
    elif name == 'lh':
        mc |= 0b001 << 12
    elif name == 'lw':
        mc |= 0b010 << 12
    elif name == 'lbu':
        mc |= 0b100 << 12
    elif name == 'lhu':
        mc |= 0b101 << 12
    mc |= (rs1 & 0x1F) << 15
    mc |= (imm & 0xFFF) << 20
    return bin(mc)[2:].zfill(32)

# store imm[11:5] rs2 rs1 funct3 imm[4:0] 0100011
def store(instr: tuple, addr: int, tags: dict) -> str:
    name = instr[0]
    rs2 = reg2idx(instr[1])
    imm = int(instr[2])
    rs1 = reg2idx(instr[3])

    mc = 0b0100011
    mc |= (imm & 0xF) << 7 # [4:0]
    if name == 'sb':
        mc |= 0b000 << 12
    elif name == 'sh':
        mc |= 0b001 << 12
    elif name == 'sw':
        mc |= 0b010 << 12
    mc |= (rs1 & 0x1F) << 15
    mc |= (rs2 & 0x1F) << 20
    mc |= ((imm & 0xFE0) >> 5) << 25
    return bin(mc)[2:].zfill(32)

# arith_i imm[11:0] rs1 funct3 rd 0010011
def arith_i(instr: tuple, addr: int, tags: dict) -> str:
    name = instr[0]
    rd = reg2idx(instr[1])
    rs1 = reg2idx(instr[2])
    imm = int(instr[3])

    mc = 0b0010011
    mc |= (rd & 0x1F) << 7
    mc |= (rs1 & 0x1F) << 15
    if name == 'addi':
        mc |= 0b000 << 12
        mc |= (imm & 0xFFF) << 20
    elif name == 'slti':
        mc |= 0b010 << 12
        mc |= (imm & 0xFFF) << 20
    elif name == 'sltiu':
        mc |= 0b011 << 12
        mc |= (imm & 0xFFF) << 20
    elif name == 'xori':
        mc |= 0b100 << 12
        mc |= (imm & 0xFFF) << 20
    elif name == 'ori':
        mc |= 0b110 << 12
        mc |= (imm & 0xFFF) << 20
    elif name == 'andi':
        mc |= 0b111 << 12
        mc |= (imm & 0xFFF) << 20
    elif name == 'slli':
        mc |= 0b001 << 12
        mc |= (imm & 0x1F) << 20 # shamt
        mc |= 0b0000000 << 25
    elif name == 'srli':
        mc |= 0b101 << 12
        mc |= (imm & 0x1F) << 20 # shamt
        mc |= 0b0000000 << 25
    elif name == 'srai':
        mc |= 0b101 << 12
        mc |= (imm & 0x1F) << 20 # shamt
        mc |= 0b0100000 << 25
    return bin(mc)[2:].zfill(32)

# arith funct7 rs2 rs1 funct3 rd 0110011
def arith(instr: tuple, addr: int, tags: dict) -> str:
    name = instr[0]
    rd = reg2idx(instr[1])
    rs1 = reg2idx(instr[2])
    rs2 = reg2idx(instr[3])

    mc = 0b0110011
    mc |= (rd & 0x1F) << 7
    mc |= (rs1 & 0x1F) << 15
    mc |= (rs2 & 0x1F) << 20
    if name == 'add':
        mc |= 0b000 << 12
        mc |= 0b0000000 << 25
    elif name == 'sub':
        mc |= 0b000 << 12
        mc |= 0b0100000 << 25
    elif name == 'sll':
        mc |= 0b001 << 12
        mc |= 0b0000000 << 25
    elif name == 'slt':
        mc |= 0b010 << 12
        mc |= 0b0000000 << 25
    elif name == 'sltu':
        mc |= 0b011 << 12
        mc |= 0b0000000 << 25
    elif name == 'xor':
        mc |= 0b100 << 12
        mc |= 0b0000000 << 25
    elif name == 'srl':
        mc |= 0b101 << 12
        mc |= 0b0000000 << 25
    elif name == 'sra':
        mc |= 0b101 << 12
        mc |= 0b0100000 << 25
    elif name == 'or':
        mc |= 0b110 << 12
        mc |= 0b0000000 << 25
    elif name == 'and':
        mc |= 0b111 << 12
        mc |= 0b0000000 << 25
    return bin(mc)[2:].zfill(32)

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
