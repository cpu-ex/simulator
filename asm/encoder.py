# code

from utils import *

######################################## code ########################################

class Code(object):

    def __init__(self, tokenizedCode: tuple, forSim: bool=True) -> None:
        self.forSim = forSim

        self.name = None
        self.rd = None
        self.rs1 = None
        self.rs2 = None
        self.imm = None
        self.tag = None

        self.assumedLength = 1
        self.actualLength = 1
    
    def isDirec(self) -> bool: return isinstance(self, Direc)
    def isTag(self) -> bool: return isinstance(self, Tags)
    
    def optimize(self, addr: int, tags: dict) -> list: return [self]
    def finalize(self, addr: int, tags: dict) -> list: return [self]
    def encode(self) -> list: pass
    def __str__(self) -> str: pass

######################################## tags ########################################

class Tags(Code):

    def __init__(self, tokenizedCode: tuple, forSim: bool=True) -> None:
        super().__init__(tokenizedCode, forSim=forSim)
        self.name = tokenizedCode[0]
        self.assumedLength = 0
        self.actualLength = 0
    
    def encode(self):
        return []
    
    def __str__(self) -> str:
        return f'{self.name}'

######################################## direc ########################################

class Direc(Code):

    def __init__(self, tokenizedCode: tuple, forSim: bool = True) -> None:
        self.name = tokenizedCode[0]
        self.data = list()
        if self.name == 'globl':
            self.tag = tokenizedCode[1]
        elif self.name == 'byte':
            self.data = pack2word([imm2int(x) & 0xFF for x in tokenizedCode[1].split(',')], 1)
        elif self.name == 'half':
            self.data = pack2word([imm2int(x) & 0xFFFF for x in tokenizedCode[1].split(',')], 2)
        elif self.name == 'word':
            self.data = [imm2int(x) & 0xFFFFFFFF for x in tokenizedCode[1].split(',')]
        elif self.name == 'float':
            self.data = [fimm2int(x) & 0xFFFFFFFF for x in tokenizedCode[1].split(',')]
        elif self.name not in ['data', 'text']:
            raise RuntimeError(f'unsupported directive \'{self.name}\'')
        self.assumedLength = len(self.data)
    
    def isStartDirec(self) -> bool:
        return self.name == 'globl'
    
    def isDataDirec(self) -> bool:
        return self.name == 'data'
    
    def isCodeDirec(self) -> bool:
        return self.name == 'text'
    
    def __str__(self) -> str:
        return f'{self.name} ' + ', '.join(hex(d) for d in self.data)

######################################## lui ########################################

class Lui(Code):

    # lui rd, imm
    def __init__(self, tokenizedCode: tuple, forSim: bool=True) -> None:
        super().__init__(tokenizedCode, forSim=forSim)
        self.name = tokenizedCode[0].lower()
        self.rd = tokenizedCode[1]
        self.imm = tokenizedCode[2]

    # lui imm[31:12] rd[5] 0110111
    def encode(self) -> list:
        rd = reg2idx(self.rd)
        imm = imm2int(self.imm)
        checkImm(imm, 32, True)

        mc = 0b0110111
        mc |= (rd & 0x1F) << 7
        mc |= ((imm >> 12) & 0xFFFFF) << 12
        return [mc]
    
    def __str__(self) -> str:
        rd = idx2reg(reg2idx(self.rd))
        imm = imm2int(self.imm)
        return f'{self.name} {rd}, {imm >> 12}'

######################################## auipc ########################################

class Auipc(Code):

    # auipc rd, imm
    def __init__(self, tokenizedCode: tuple, forSim: bool=True) -> None:
        super().__init__(tokenizedCode, forSim=forSim)
        self.name = tokenizedCode[0].lower()
        self.rd = tokenizedCode[1]
        self.imm = tokenizedCode[2]

    # auipc imm[31:12] rd[5] 0010111
    def encode(self) -> list:
        rd = reg2idx(self.rd)
        imm = imm2int(self.imm)
        checkImm(imm, 32, True)
    
        mc = 0b0010111
        mc |= (rd & 0x1F) << 7
        mc |= ((imm >> 12) & 0xFFFFF) << 12
        return [mc]
    
    def __str__(self):
        rd = idx2reg(reg2idx(self.rd))
        imm = self.imm
        return f'{self.name} {rd}, {imm}'

######################################## jal ########################################

class Jal(Code):

    # jal rd, tag
    def __init__(self, tokenizedCode: tuple, forSim: bool=True) -> None:
        super().__init__(tokenizedCode, forSim=forSim)
        self.name = tokenizedCode[0].lower()
        self.rd = tokenizedCode[1]
        self.tag = tokenizedCode[2]
    
    def finalize(self, addr: int, tags: dict) -> list:
        self.imm = tag2offset(self.tag, tags, addr)
        return [self]
    
    # jal imm[20,10:1,11,19:12] rd 110111
    def encode(self) -> list:
        rd = reg2idx(self.rd)
        imm = self.imm
        checkImm(imm, 21, True)

        mc = 0b1101111
        mc |= (rd & 0x1F) << 7
        mc |= ((imm & 0x000FF000) >> 12) << 12 # 19:12
        mc |= ((imm & 0x00000800) >> 11) << 20 # 11
        mc |= ((imm & 0x000007FE) >>  1) << 21 # 10:1
        mc |= ((imm & 0x00100000) >> 20) << 31 # 20
        return [mc]

    def __str__(self) -> str:
        rd = idx2reg(reg2idx(self.rd))
        imm = self.imm
        return f'{self.name} {rd}, {imm}'

######################################## jalr ########################################

class Jalr(Code):

    # jalr rd, offset(rs1)
    def __init__(self, tokenizedCode: tuple, forSim: bool=True) -> None:
        super().__init__(tokenizedCode, forSim=forSim)
        self.name = tokenizedCode[0].lower()
        self.rd = tokenizedCode[1]
        self.imm = tokenizedCode[2]
        self.rs1 = tokenizedCode[3]

    # jalr imm[11:0] rs1 000 rd 1100111
    def encode(self) -> list:
        rd = reg2idx(self.rd)
        imm = imm2int(self.imm)
        rs1 = reg2idx(self.rs1)
        checkImm(imm, 12, True)

        mc = 0b1100111
        mc |= (rd & 0x1F) << 7
        mc |= 0b000 << 12
        mc |= (rs1 & 0x1F) << 15
        mc |= (imm & 0xFFF) << 20
        return [mc]

    def __str__(self) -> str:
        rd = idx2reg(reg2idx(self.rd))
        imm = self.imm
        rs1 = idx2reg(reg2idx(self.rs1))
        return f'{self.name} {rd}, {imm}({rs1})'

######################################## branch ########################################

class Branch(Code):

    # branch rs1, rs2, tag
    def __init__(self, tokenizedCode: tuple, forSim: bool=True) -> None:
        super().__init__(tokenizedCode, forSim=forSim)
        self.name = tokenizedCode[0].lower()
        self.rs1 = tokenizedCode[1]
        self.rs2 = tokenizedCode[2]
        self.tag = tokenizedCode[3]
        self.assumedLength = 2
    
    def optimize(self, addr: int, tags: dict) -> list:
        name = self.name
        imm = tag2offset(self.tag, tags, addr)

        try:
            checkImm(imm, 13, True)
            self.actualLength = 1
            return [self]
        except RuntimeError:
            oppositeDict = {
                'beq': 'bne', 'bne': 'beq',
                'blt': 'bge', 'bge': 'blt'
            }
            oldTag = self.tag
            newTag = f'additional_branch_tag_{inc()}'
            newName = oppositeDict[name]
            self.actualLength = 2
            return [
                Branch((newName, self.rs1, self.rs2, newTag)),
                Jal(('jal', 'zero', oldTag)),
                Tags((newTag,))
            ]
    
    def finalize(self, addr: int, tags: dict) -> list:
        self.imm = tag2offset(self.tag, tags, addr)
        return [self]

    # branch imm[12,10:5] rs2 rs1 funct3 imm[4:1,11] 1100011
    def encode(self) -> list:
        name = self.name
        rs1 = reg2idx(self.rs1)
        rs2 = reg2idx(self.rs2)
        imm = self.imm
        checkImm(imm, 13, True)

        mc = 0b1100011
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
        else:
            # not suppose to be here
            raise RuntimeError(f'unrecognizable branch type \'{name}\'')
        mc |= (rs1 & 0x1F) << 15
        mc |= (rs2 & 0x1F) << 20
        mc |= ((imm & 0x000007E0) >>  5) << 25 # 10:5
        mc |= ((imm & 0x00001000) >> 12) << 31 # 12
        return [mc]
    
    def __str__(self) -> str:
        rs1 = idx2reg(reg2idx(self.rs1))
        rs2 = idx2reg(reg2idx(self.rs2))
        imm = self.imm
        return f'{self.name} {rs1}, {rs2}, {imm}'

######################################## load ########################################

class Load(Code):

    # load rd, offset(rs1)
    def __init__(self, tokenizedCode: tuple, forSim: bool=True) -> None:
        super().__init__(tokenizedCode, forSim=forSim)
        self.name = tokenizedCode[0].lower()
        self.rd = tokenizedCode[1]
        self.imm = tokenizedCode[2]
        self.rs1 = tokenizedCode[3]

    # load imm[11:0] rs1 funct3 rd 0000011
    def encode(self) -> list:
        name = self.name
        rd = reg2idx(self.rd)
        imm = imm2int(self.imm)
        rs1 = reg2idx(self.rs1)
        checkImm(imm, 12, True)

        mc = 0b0000011
        mc |= (rd & 0x1F) << 7
        if name == 'lw':
            mc |= 0b010 << 12
        else:
            # not suppose to be here
            raise RuntimeError(f'unrecognizable load type \'{name}\'')
        mc |= (rs1 & 0x1F) << 15
        mc |= (imm & 0xFFF) << 20
        return [mc]
    
    def __str__(self) -> str:
        rd = idx2reg(reg2idx(self.rd))
        imm = self.imm
        rs1 = idx2reg(reg2idx(self.rs1))
        return f'{self.name} {rd}, {imm}({rs1})'

######################################## store ########################################

class Store(Code):

    # store rs2, offset(rs1)
    def __init__(self, tokenizedCode: tuple, forSim: bool=True) -> None:
        super().__init__(tokenizedCode, forSim=forSim)
        self.name = tokenizedCode[0].lower()
        self.rs2 = tokenizedCode[1]
        self.imm = tokenizedCode[2]
        self.rs1 = tokenizedCode[3]
    
    # store imm[11:5] rs2 rs1 funct3 imm[4:0] 0100011
    def encode(self) -> list:
        name = self.name
        rs2 = reg2idx(self.rs2)
        imm = imm2int(self.imm)
        rs1 = reg2idx(self.rs1)
        checkImm(imm, 12, True)

        mc = 0b0100011
        mc |= (imm & 0x1F) << 7 # [4:0]
        if name == 'sw':
            mc |= 0b010 << 12
        elif name == 'swi':
            mc |= 0b011 << 12
        else:
            # not suppose to be here
            raise RuntimeError(f'unrecognizable store type \'{name}\'')
        mc |= (rs1 & 0x1F) << 15
        mc |= (rs2 & 0x1F) << 20
        mc |= ((imm & 0xFE0) >> 5) << 25
        return [mc]
    
    def __str__(self) -> str:
        rs2 = idx2reg(reg2idx(self.rs2))
        imm = self.imm
        rs1 = idx2reg(reg2idx(self.rs1))
        return f'{self.name} {rs2}, {imm}({rs1})'

######################################## arith-i ########################################

class Arith_i(Code):

    # arith_i rd, rs1, imm
    def __init__(self, tokenizedCode: tuple, forSim: bool=True) -> None:
        super().__init__(tokenizedCode, forSim=forSim)
        self.name = tokenizedCode[0].lower()
        self.rd = tokenizedCode[1]
        self.rs1 = tokenizedCode[2]
        self.imm = tokenizedCode[3]

    # arith_i imm[11:0] rs1 funct3 rd 0010011
    def encode(self) -> list:
        name = self.name
        rd = reg2idx(self.rd)
        rs1 = reg2idx(self.rs1)
        imm = imm2int(self.imm)
        checkImm(imm, 12, True)

        mc = 0b0010011
        mc |= (rd & 0x1F) << 7
        mc |= (rs1 & 0x1F) << 15
        if name == 'addi':
            mc |= 0b000 << 12
            mc |= (imm & 0xFFF) << 20
        elif name == 'slti':
            mc |= 0b010 << 12
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
        else:
            # not suppose to be here
            raise RuntimeError(f'unrecognizable arith-i type \'{name}\'')
        return [mc]

    def __str__(self) -> str:
        rd = idx2reg(reg2idx(self.rd))
        rs1 = idx2reg(reg2idx(self.rs1))
        imm = self.imm
        return f'{self.name} {rd}, {rs1}, {imm}'

######################################## arith ########################################

class Arith(Code):

    # arith rd, rs1, rs2
    def __init__(self, tokenizedCode: tuple, forSim: bool=True) -> None:
        super().__init__(tokenizedCode, forSim=forSim)
        self.name = tokenizedCode[0].lower()
        self.rd = tokenizedCode[1]
        self.rs1 = tokenizedCode[2]
        self.rs2 = tokenizedCode[3]

    # arith funct7 rs2 rs1 funct3 rd 0110011
    def encode(self) -> list:
        name = self.name
        rd = reg2idx(self.rd)
        rs1 = reg2idx(self.rs1)
        rs2 = reg2idx(self.rs2)

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
        else:
            # not suppose to be here
            raise RuntimeError(f'unrecognizable arith type \'{name}\'')
        return [mc]
    
    def __str__(self) -> str:
        rd = idx2reg(reg2idx(self.rd))
        rs1 = idx2reg(reg2idx(self.rs1))
        rs2 = idx2reg(reg2idx(self.rs2))
        return f'{self.name} {rd}, {rs1}, {rs2}'

######################################## ebreak ########################################

class Ebreak(Code):

    def __init__(self, tokenizedCode: tuple, forSim: bool) -> None:
        super().__init__(tokenizedCode, forSim=forSim)
        self.name = tokenizedCode[0].lower()
        self.imm = tokenizedCode[1]
    
    def optimize(self, addr: int, tags: dict) -> list:
        imm = imm2int(self.imm)
        checkImm(imm, 12, False)
        if (not self.forSim) and (imm != 0):
            return [Pseudo_nop(('nop'))]
        else:
            return [self]

    def encode(self) -> list:
        imm = imm2int(self.imm)
        checkImm(imm, 12, False)

        if self.forSim:
            mc = 0b1110011
            mc |= (imm & 0xFFF) << 20
            return [mc]
        elif imm == 0:
            # infinity loop
            return [0b1101111]
        else:
            raise RuntimeError(f'ebreak {imm} is invalid for fpga.')

    def __str__(self) -> str:
        return f'{self.name} {self.imm}'

######################################## f-load ########################################

class F_load(Code):

    # flw rd, offset(rs1)
    def __init__(self, tokenizedCode: tuple, forSim: bool=True) -> None:
        super().__init__(tokenizedCode, forSim=forSim)
        self.name = tokenizedCode[0].lower()
        self.rd = tokenizedCode[1]
        self.imm = tokenizedCode[2]
        self.rs1 = tokenizedCode[3]
    
    # f-load imm[11:0] rs1 010 rd 0000111
    def encode(self) -> list:
        rd = reg2idx(self.rd)
        imm = imm2int(self.imm)
        rs1 = reg2idx(self.rs1)
        checkImm(imm, 12, True)

        mc = 0b0000111
        mc |= (rd & 0x1F) << 7
        mc |= 0b010 << 12
        mc |= (rs1 & 0x1F) << 15
        mc |= (imm & 0xFFF) << 20
        return [mc]
    
    def __str__(self) -> str:
        rd = idx2reg(reg2idx(self.rd), type='f')
        imm = self.imm
        rs1 = idx2reg(reg2idx(self.rs1), type='f')
        return f'{self.name} {rd}, {imm}({rs1})'

######################################## f-store ########################################

class F_store(Code):

    # fsw rs2, offset(rs1)
    def __init__(self, tokenizedCode: tuple, forSim: bool=True) -> None:
        super().__init__(tokenizedCode, forSim=forSim)
        self.name = tokenizedCode[0].lower()
        self.rs2 = tokenizedCode[1]
        self.imm = tokenizedCode[2]
        self.rs1 = tokenizedCode[3]
    
    # f-store imm[11:5] rs2 rs1 010 imm[4:0] 0100111
    def encode(self) -> list:
        rs2 = reg2idx(self.rs2)
        imm = imm2int(self.imm)
        rs1 = reg2idx(self.rs1)
        checkImm(imm, 12, True)

        mc = 0b0100111
        mc |= (imm & 0x1F) << 7 # [4:0]
        mc |= 0b010 << 12
        mc |= (rs1 & 0x1F) << 15
        mc |= (rs2 & 0x1F) << 20
        mc |= ((imm & 0xFE0) >> 5) << 25
        return [mc]
    
    def __str__(self) -> str:
        rs2 = idx2reg(reg2idx(self.rs2), type='f')
        imm = self.imm
        rs1 = idx2reg(reg2idx(self.rs1), type='f')
        return f'{self.name} {rs2}, {imm}({rs1})'

######################################## f-arith ########################################

class F_arith(Code):

    # f-arith rd, rs1, rs2
    def __init__(self, tokenizedCode: tuple, forSim: bool=True) -> None:
        super().__init__(tokenizedCode, forSim=forSim)
        self.name = tokenizedCode[0].lower()
        self.rd = tokenizedCode[1]
        self.rs1 = tokenizedCode[2]
        try:
            self.rs2 = tokenizedCode[3]
        except IndexError:
            self.rs2 = 'zero'

    # f-arith funct7 rs2/funct5 rs1 rm/funct3 rd 1010011
    def encode(self) -> list:
        name = self.name
        rd = reg2idx(self.rd)
        rs1 = reg2idx(self.rs1)
        rs2 = reg2idx(self.rs2)
        
        mc = 0b1010011
        mc |= (rd & 0x1F) << 7
        mc |= (rs1 & 0x1F) << 15
        # rm field ignored
        if name == 'fmv.x.w':
            mc |= 0b000 << 12
            mc |= 0b1110000 << 25
        elif name == 'fmv.w.x':
            mc |= 0b000 << 12
            mc |= 0b1111000 << 25
        elif name == 'fadd':
            mc |= 0b0000000 << 25
        elif name == 'fsub':
            mc |= 0b0000100 << 25
        elif name == 'fmul':
            mc |= 0b0001000 << 25
        elif name == 'fdiv':
            mc |= 0b0001100 << 25
        elif name == 'fsqrt':
            rs2 = 0b00000
            mc |= 0b0101100 << 25
        elif name == 'feq':
            mc |= 0b010 << 12
            mc |= 0b1010000 << 25
        elif name == 'flt':
            mc |= 0b001 << 12
            mc |= 0b1010000 << 25
        elif name == 'fle':
            mc |= 0b000 << 12
            mc |= 0b1010000 << 25
        elif name == 'fcvt.s.w':
            rs2 = 0b00000
            mc |= 0b1101000 << 25
        elif name == 'fcvt.w.s':
            rs2 = 0b00000
            mc |= 0b1100000 << 25
        elif name == 'fcvt.s.wu':
            rs2 = 0b00001
            mc |= 0b1101000 << 25
        elif name == 'fcvt.wu.s':
            rs2 = 0b00001
            mc |= 0b1100000 << 25
        elif name == 'fsgnj':
            mc |= 0b000 << 12
            mc |= 0b0010000 << 25
        elif name == 'fsgnjn':
            mc |= 0b001 << 12
            mc |= 0b0010000 << 25
        elif name == 'fsgnjx':
            mc |= 0b010 << 12
            mc |= 0b0010000 << 25
        else:
            # not suppose to be here
            raise RuntimeError(f'unrecognizable arith type \'{name}\'')
        mc |= (rs2 & 0x1F) << 20
        return [mc]
    
    def __str__(self) -> str:
        rd = idx2reg(reg2idx(self.rd), type='f')
        rs1 = idx2reg(reg2idx(self.rs1), type='f')
        rs2 = idx2reg(reg2idx(self.rs2), type='f')
        if self.name == 'fsqrt':
            return f'{self.name} {rd}, {rs1}'
        else:
            return f'{self.name} {rd}, {rs1}, {rs2}'

######################################## f-branch ########################################

class F_branch(Code):

    # f-branch rs1, rs2, tag
    def __init__(self, tokenizedCode: tuple, forSim: bool = True) -> None:
        super().__init__(tokenizedCode, forSim=forSim)
        self.name = tokenizedCode[0].lower()
        self.rs1 = tokenizedCode[1]
        self.rs2 = tokenizedCode[2]
        self.tag = tokenizedCode[3]
        self.assumedLength = 3
    
    def optimize(self, addr: int, tags: dict) -> list:
        name = self.name
        imm = tag2offset(self.tag, tags, addr)

        try:
            checkImm(imm, 13, True)
            self.actualLength = 1
            return [self]
        except RuntimeError:
            oldTag = self.tag
            newTag = f'additional_branch_tag_{inc()}'
            if name == 'bfeq':
                self.actualLength = 3
                return [
                    F_arith(('feq', 'x4', self.rs1, self.rs2)),
                    F_branch(('bfeq', 'x4', 'zero', newTag)),
                    Jal(('jal', 'zero', oldTag)),
                    Tags((newTag,))
                ]
            elif name in ['bflt', 'bfle']:
                newName = 'bflt' if name == 'bfle' else 'bfle'
                self.actualLength = 2
                return [
                    F_branch((newName, self.rs2, self.rs1, newTag)),
                    Jal(('jal', 'zero', oldTag)),
                    Tags((newTag,))
                ]
            else:
                # not suppose to be here
                raise RuntimeError(f'unrecognizable f-branch type \'{name}\'')

    def finalize(self, addr: int, tags: dict) -> list:
        self.imm = tag2offset(self.tag, tags, addr)
        return [self]
    
    # f-branch imm[12,10:5] rs2 rs1 funct3 imm[4:1,11] 1100001
    def encode(self) -> list:
        name = self.name
        rs1 = reg2idx(self.rs1)
        rs2 = reg2idx(self.rs2)
        imm = self.imm
        checkImm(imm, 13, True)

        mc = 0b1100001
        mc |= ((imm & 0x00000800) >> 11) << 7 # 11
        mc |= ((imm & 0x0000001E) >>  1) << 8 # 4:1
        if name == 'bfeq':
            mc |= 0b000 << 12
        elif name == 'bfle':
            mc |= 0b001 << 12
        elif name == 'bflt':
            mc |= 0b010 << 12
        else:
            # not suppose to be here
            raise RuntimeError(f'unrecognizable branch type \'{name}\'')
        mc |= (rs1 & 0x1F) << 15
        mc |= (rs2 & 0x1F) << 20
        mc |= ((imm & 0x000007E0) >>  5) << 25 # 10:5
        mc |= ((imm & 0x00001000) >> 12) << 31 # 12
        return [mc]
    
    def __str__(self) -> str:
        rs1 = idx2reg(reg2idx(self.rs1), type='f')
        rs2 = idx2reg(reg2idx(self.rs2), type='f')
        imm = self.imm
        return f'{self.name} {rs1}, {rs2}, {imm}'

######################################## vector ########################################

class Vector(Code):

    def __init__(self, tokenizedCode: tuple, forSim: bool = True) -> None:
        super().__init__(tokenizedCode, forSim)
        self.name = tokenizedCode[0].lower()
        self.r2 = tokenizedCode[1]
        self.r3 = tokenizedCode[2]
        self.r4 = tokenizedCode[3]
        self.r5 = tokenizedCode[4]
        self.imm = tokenizedCode[5]
        self.r1 = tokenizedCode[6]
        self.mask = tokenizedCode[7]

        self.assumedLength = 2
        self.actualLength = 2
    
    # vec r2, r3, r4, r5, imm(r1), mask
    def encode(self) -> list:
        name = self.name
        r1 = reg2idx(self.r1, aligned=True)
        r2 = reg2idx(self.r2, aligned=True)
        r3 = reg2idx(self.r3, aligned=True)
        r4 = reg2idx(self.r4, aligned=True)
        r5 = reg2idx(self.r5, aligned=True)
        imm = imm2int(self.imm)
        mask = int(self.mask, base=2)
        checkImm(imm, 12, True)
        checkImm(mask, 4, False)

        mc1 = 0b1000000 if name == 'vlw' else 0b1000010
        mc1 |= (mask & 0xF) << 7
        mc1 |= (r1 & 0x1F) << 15
        mc1 |= (imm & 0xFFF) << 20
        mc2 = 0b1100000 if name == 'vlw' else 0b1100010
        mc2 |= (r5 & 0x3F) << 8
        mc2 |= (r4 & 0x3F) << 14
        mc2 |= (r3 & 0x3F) << 20
        mc2 |= (r2 & 0x3F) << 26

        return [mc1, mc2]
    
    def __str__(self) -> str:
        r1 = idx2reg(reg2idx(self.r1, aligned=True))
        r2 = idx2reg(reg2idx(self.r2, aligned=True))
        r3 = idx2reg(reg2idx(self.r3, aligned=True))
        r4 = idx2reg(reg2idx(self.r4, aligned=True))
        r5 = idx2reg(reg2idx(self.r5, aligned=True))
        imm = self.imm
        mask = self.mask
        return f'{self.name} {r2}, {r3}, {r4}, {r5}, {imm}({r1}), {mask}'

######################################## pseudo-nop ########################################

class Pseudo_nop(Code):

    def finalize(self, addr: int, tags: dict) -> list:
        return [Arith_i(('addi', 'zero', 'zero', '0'))]
    
    def __str__(self) -> str:
        return f'nop'

######################################## pseudo-li ########################################

class Pseudo_li(Code):

    def __init__(self, tokenizedCode: tuple, forSim: bool=True) -> None:
        super().__init__(tokenizedCode, forSim=forSim)
        self.rd = tokenizedCode[1]
        self.imm = tokenizedCode[2]
        self.assumedLength = 2
    
    def optimize(self, addr: int, tags: dict) -> list:
        hi, lo = getHiLo(imm2int(self.imm))
        if hi == 0:
            self.actualLength = 1
            return [Arith_i(('addi', self.rd, 'zero', str(lo)))]
        elif lo == 0:
            self.actualLength = 1
            return [Lui(('lui', self.rd, str(hi)))]
        else:
            self.actualLength = 2
            return [Lui(('lui', self.rd, str(hi))), Arith_i(('addi', self.rd, self.rd, str(lo)))]
    
    def __str__(self) -> str:
        return f'li {self.rd}, {self.imm}'

######################################## pseudo-la ########################################

class Pseudo_la(Code):

    def __init__(self, tokenizedCode: tuple, forSim: bool=True) -> None:
        super().__init__(tokenizedCode, forSim=forSim)
        self.rd = tokenizedCode[1]
        self.tag = tokenizedCode[2]
        self.assumedLength = 2
    
    def optimize(self, addr: int, tags: dict) -> list:
        try:
            imm = tag2imm(self.tag, tags)
            # check if the address of tag is small enough to fit in addi
            checkImm(imm, 12, True)
            self.actualLength = 1
        except RuntimeError:
            self.actualLength = 2
        return [self]
    
    def finalize(self, addr: int, tags: dict) -> list:
        try:
            imm = tag2imm(self.tag, tags)
            checkImm(imm, 12, True)
            # the address of tag is small enough to fit in addi
            return [Arith_i(('addi', self.rd, 'zero', str(imm)))]
        except RuntimeError:
            offset = tag2offset(self.tag, tags, addr)
            hi, lo = getHiLo(offset)
            return [
                Auipc(('auipc', self.rd, str(hi))),
                Arith_i(('addi', self.rd, self.rd, str(lo)))
            ]
    
    def __str__(self) -> str:
        return f'la {self.rd}, {self.tag}'

######################################## pseudo-not ########################################

class Pseudo_not(Code):

    def __init__(self, tokenizedCode: tuple, forSim: bool=True) -> None:
        super().__init__(tokenizedCode, forSim=forSim)
        self.rd = tokenizedCode[1]
        self.rs1 = tokenizedCode[2]

    def finalize(self, addr: int, tags: dict) -> list:
        return [Arith_i(('xori', self.rd, self.rs1, '-1'))]
    
    def __str__(self) -> str:
        return f'not {self.rd}, {self.rs1}'

######################################## pseudo-mv ########################################

class Pseudo_mv(Code):

    def __init__(self, tokenizedCode: tuple, forSim: bool=True) -> None:
        super().__init__(tokenizedCode, forSim=forSim)
        self.rd = tokenizedCode[1]
        self.rs1 = tokenizedCode[2]
    
    def finalize(self, addr: int, tags: dict) -> list:
        return [Arith_i(('addi', self.rd, self.rs1, '0'))]
    
    def __str__(self) -> str:
        return f'mv {self.rd}, {self.rs1}'

######################################## pseudo-j ########################################

class Pseudo_j(Code):

    def __init__(self, tokenizedCode: tuple, forSim: bool=True) -> None:
        super().__init__(tokenizedCode, forSim=forSim)
        self.tag = tokenizedCode[1]
    
    def optimize(self, addr: int, tags: dict) -> list:
        return [Jal(('jal', 'zero', self.tag))]
    
    def __str__(self) -> str:
        return f'j {self.tag}'

######################################## pseudo-jal ########################################

class Pseudo_jal(Code):

    def __init__(self, tokenizedCode: tuple, forSim: bool=True) -> None:
        super().__init__(tokenizedCode, forSim=forSim)
        self.tag = tokenizedCode[1]
    
    def optimize(self, addr: int, tags: dict) -> list:
        return [Jal(('jal', 'ra', self.tag))]
    
    def __str__(self) -> str:
        return f'jal {self.tag}'

######################################## pseudo-jalr ########################################

class Pseudo_jalr(Code):

    def __init__(self, tokenizedCode: tuple, forSim: bool=True) -> None:
        super().__init__(tokenizedCode, forSim=forSim)
        self.rs1 = tokenizedCode[1]
    
    def finalize(self, addr: int, tags: dict) -> list:
        return [Jalr(('jalr', 'ra', '0', self.rs1))]

    def __str__(self) -> str:
        return f'jalr {self.rs1}'

######################################## pseudo-ret ########################################

class Pseudo_ret(Code):

    def finalize(self, addr: int, tags: dict) -> list:
        return [Jalr(('jalr', 'zero', '0', 'ra'))]
    
    def __str__(self) -> str:
        return f'ret'

######################################## pseudo-call ########################################

class Pseudo_call(Code):

    def __init__(self, tokenizedCode: tuple, forSim: bool=True) -> None:
        super().__init__(tokenizedCode, forSim=forSim)
        self.tag = tokenizedCode[1]
        self.assumedLength = 2
    
    def optimize(self, addr: int, tags: dict) -> list:
        try:
            offset = tag2offset(self.tag, tags, addr)
            # check if the address of tag is small enough to fit in jal
            checkImm(offset, 21, True)
            self.actualLength = 1
        except RuntimeError:
            self.actualLength = 2
        return [self]
    
    def finalize(self, addr: int, tags: dict) -> list:
        offset = tag2offset(self.tag, tags, addr)
        try:
            checkImm(offset, 21, True)
            # the address of tag is small enough to fit in jal
            jal = Jal(('jal', 'ra', self.tag))
            jal.imm = offset
            return [jal]
        except RuntimeError:
            hi, lo = getHiLo(offset)
            return [
                Auipc(('auipc', 't1', str(hi))),
                Jalr(('jalr', 'ra', str(lo), 't1'))
            ]

    def __str__(self) -> str:
        return f'call {self.tag}'

######################################## pseudo-tail ########################################

class Pseudo_tail(Code):

    def __init__(self, tokenizedCode: tuple, forSim: bool=True) -> None:
        super().__init__(tokenizedCode, forSim=forSim)
        self.tag = tokenizedCode[1]
        self.assumedLength = 2
        self.actualLength = 2
    
    def finalize(self, addr: int, tags: dict) -> list:
        offset = tag2offset(self.tag, tags, addr)
        hi, lo = getHiLo(offset)
        return [
            Auipc(('auipc', 't1', str(hi))),
            Jalr(('jalr', 'zero', str(lo), 't1'))
        ]

    def __str__(self) -> str:
        return f'tail {self.tag}'

######################################## pseudo-ebreak ########################################

class Pseudo_ebreak(Code):
    
    def finalize(self, addr: int, tags: dict) -> list:
        return [Ebreak(('ebreak', '0'), self.forSim)]
    
    def __str__(self) -> str:
        return f'ebreak'

######################################## pseudo-fmv ########################################

class Pseudo_fmv(Code):

    def __init__(self, tokenizedCode: tuple, forSim: bool=True) -> None:
        super().__init__(tokenizedCode, forSim=forSim)
        self.rd = tokenizedCode[1]
        self.rs1 = tokenizedCode[2]
    
    def finalize(self, addr: int, tags: dict) -> list:
        return [F_arith(('fsgnj', self.rd, self.rs1, self.rs1))]
    
    def __str__(self) -> str:
        return f'fmv {self.rd}, {self.rs1}'

######################################## pseudo-fabs ########################################

class Pseudo_fabs(Code):

    def __init__(self, tokenizedCode: tuple, forSim: bool=True) -> None:
        super().__init__(tokenizedCode, forSim=forSim)
        self.rd = tokenizedCode[1]
        self.rs1 = tokenizedCode[2]
    
    def finalize(self, addr: int, tags: dict) -> list:
        return [F_arith(('fsgnjx', self.rd, self.rs1, self.rs1))]
    
    def __str__(self) -> str:
        return f'fabs {self.rd}, {self.rs1}'

######################################## pseudo-fneg ########################################

class Pseudo_fneg(Code):

    def __init__(self, tokenizedCode: tuple, forSim: bool=True) -> None:
        super().__init__(tokenizedCode, forSim=forSim)
        self.rd = tokenizedCode[1]
        self.rs1 = tokenizedCode[2]
    
    def finalize(self, addr: int, tags: dict) -> list:
        return [F_arith(('fsgnjn', self.rd, self.rs1, self.rs1))]
    
    def __str__(self) -> str:
        return f'fneg {self.rd}, {self.rs1}'
