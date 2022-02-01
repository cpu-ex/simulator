# decoder

import re
from encoder import *

class Block(object):

    decoders = {
        # tag
        'TAG': (re.compile(r'(\S+):'), Tags),
        # directive
        'DIREC-DATA': (re.compile(r'\.(\S+?)\s+(.+)'), Direc),
        'DIREC-PLAIN': (re.compile(r'\.(\S+)'), Direc),


        # RV32I
        # pc rd, imm
        'LUI': (re.compile(r'(lui)\s+(\S+),\s*(-?\S+)'), Lui),
        'AUIPC': (re.compile(r'(auipc)\s+(\S+),\s*(-?\S+)'), Auipc),
        # jal rd, tag
        'JAL': (re.compile(r'(jal)\s+(\S+),\s*(\S+)'), Jal),
        # jalr rd, offset(rs1)
        'JALR': (re.compile(r'(jalr)\s+(\S+),\s*(-?\S+)\((\S+)\)'), Jalr),
        # branch rs1, rs2, tag
        'BEQ': (re.compile(r'(beq)\s+(\S+),\s*(\S+),\s*(\S+)'), Branch),
        'BNE': (re.compile(r'(bne)\s+(\S+),\s*(\S+),\s*(\S+)'), Branch),
        'BLT': (re.compile(r'(blt)\s+(\S+),\s*(\S+),\s*(\S+)'), Branch),
        'BGE': (re.compile(r'(bge)\s+(\S+),\s*(\S+),\s*(\S+)'), Branch),
        # load rd, offset(rs1)
        'LW': (re.compile(r'(lw)\s+(\S+),\s*(-?\S+)\((\S+)\)'), Load),
        # store rs2, offset(rs1)
        'SW': (re.compile(r'(sw)\s+(\S+),\s*(-?\S+)\((\S+)\)'), Store),
        # arith_i rd, rs1, imm
        'ADDI': (re.compile(r'(addi)\s+(\S+),\s*(\S+),\s*(-?\S+)'), Arith_i),
        'SLLI': (re.compile(r'(slli)\s+(\S+),\s*(\S+),\s*(-?\S+)'), Arith_i),
        'SLTI': (re.compile(r'(slti)\s+(\S+),\s*(\S+),\s*(-?\S+)'), Arith_i),
        'XORI': (re.compile(r'(xori)\s+(\S+),\s*(\S+),\s*(-?\S+)'), Arith_i),
        'SRLI': (re.compile(r'(srli)\s+(\S+),\s*(\S+),\s*(-?\S+)'), Arith_i),
        'SRAI': (re.compile(r'(srai)\s+(\S+),\s*(\S+),\s*(-?\S+)'), Arith_i),
        'ORI': (re.compile(r'(ori)\s+(\S+),\s*(\S+),\s*(-?\S+)'), Arith_i),
        'ANDI': (re.compile(r'(andi)\s+(\S+),\s*(\S+),\s*(-?\S+)'), Arith_i),
        # arith rd, rs1, rs2
        'ADD': (re.compile(r'(add)\s+(\S+),\s*(\S+),\s*(\S+)'), Arith),
        'SUB': (re.compile(r'(sub)\s+(\S+),\s*(\S+),\s*(\S+)'), Arith),
        'SLL': (re.compile(r'(sll)\s+(\S+),\s*(\S+),\s*(\S+)'), Arith),
        'SLT': (re.compile(r'(slt)\s+(\S+),\s*(\S+),\s*(\S+)'), Arith),
        'XOR': (re.compile(r'(xor)\s+(\S+),\s*(\S+),\s*(\S+)'), Arith),
        'SRL': (re.compile(r'(srl)\s+(\S+),\s*(\S+),\s*(\S+)'), Arith),
        'SRA': (re.compile(r'(sra)\s+(\S+),\s*(\S+),\s*(\S+)'), Arith),
        'OR': (re.compile(r'(or)\s+(\S+),\s*(\S+),\s*(\S+)'), Arith),
        'AND': (re.compile(r'(and)\s+(\S+),\s*(\S+),\s*(\S+)'), Arith),
        # env
        'EBREAK': (re.compile(r'(ebreak)\s+(\S+)'), Ebreak),


        # RV32F
        'FLW': (re.compile(r'(flw)\s+(\S+),\s*(-?\S+)\((\S+)\)'), F_load),
        'FSW': (re.compile(r'(fsw)\s+(\S+),\s*(-?\S+)\((\S+)\)'), F_store),
        'FMV.X.W': (re.compile(r'(fmv\.x\.w)\s+(\S+),\s*(\S+)'), F_arith),
        'FMV.W.X': (re.compile(r'(fmv\.w\.x)\s+(\S+),\s*(\S+)'), F_arith),
        'FADD': (re.compile(r'(fadd)\s+(\S+),\s*(\S+),\s*(\S+)'), F_arith),
        'FSUB': (re.compile(r'(fsub)\s+(\S+),\s*(\S+),\s*(\S+)'), F_arith),
        'FMUL': (re.compile(r'(fmul)\s+(\S+),\s*(\S+),\s*(\S+)'), F_arith),
        'FDIV': (re.compile(r'(fdiv)\s+(\S+),\s*(\S+),\s*(\S+)'), F_arith),
        'FSQRT': (re.compile(r'(fsqrt)\s+(\S+),\s*(\S+)'), F_arith),
        'FEQ': (re.compile(r'(feq)\s+(\S+),\s*(\S+),\s*(\S+)'), F_arith),
        'FLT': (re.compile(r'(flt)\s+(\S+),\s*(\S+),\s*(\S+)'), F_arith),
        'FLE': (re.compile(r'(fle)\s+(\S+),\s*(\S+),\s*(\S+)'), F_arith),
        'FCVT.S.W': (re.compile(r'(fcvt\.s\.w)\s+(\S+),\s*(\S+)'), F_arith),
        'FCVT.W.S': (re.compile(r'(fcvt\.w\.s)\s+(\S+),\s*(\S+)'), F_arith),
        'FSGNJ': (re.compile(r'(fsgnj)\s+(\S+),\s*(\S+),\s*(\S+)'), F_arith),
        'FSGNJN': (re.compile(r'(fsgnjn)\s+(\S+),\s*(\S+),\s*(\S+)'), F_arith),
        'FSGNJX': (re.compile(r'(fsgnjx)\s+(\S+),\s*(\S+),\s*(\S+)'), F_arith),


        # pseudo
        'PSEUDO-NOP': (re.compile(r'(nop)'), Pseudo_nop),
        'PSEUDO-LI': (re.compile(r'(li)\s+(\S+),\s*(-?\S+)'), Pseudo_li),
        'PSEUDO-LA': (re.compile(r'(la)\s+(\S+),\s*(\S+)'), Pseudo_la),
        'PSEUDO-NOT': (re.compile(r'(not)\s+(\S+),\s*(\S+)'), Pseudo_not),
        'PSEUDO-MV': (re.compile(r'(mv)\s+(\S+),\s*(\S+)'), Pseudo_mv),
        'PSEUDO-J': (re.compile(r'(j)\s+(\S+)'), Pseudo_j),
        'PSEUDO-JAL': (re.compile(r'(jal)\s+(\S+)'), Pseudo_jal),
        'PSEUDO-JALR': (re.compile(r'(jalr)\s+(\S+)'), Pseudo_jalr),
        'PSEUDO-RET': (re.compile(r'(ret)'), Pseudo_ret),
        'PSEUDO-CALL': (re.compile(r'(call)\s+(\S+)'), Pseudo_call),
        'PSEUDO-TAIL': (re.compile(r'(tail)\s+(\S+)'), Pseudo_tail),
        'PSEUDO-EBREAK': (re.compile(r'(ebreak)'), Pseudo_ebreak),
        'PSEUDO-FMV': (re.compile(r'(fmv)\s+(\S+),\s*(\S+)'), Pseudo_fmv),
        'PSEUDO-FABS': (re.compile(r'(fabs)\s+(\S+),\s*(\S+)'), Pseudo_fabs),
        'PSEUDO-FNEG': (re.compile(r'(fneg)\s+(\S+),\s*(\S+)'), Pseudo_fneg),
        # special
        'SWI': (re.compile(r'(swi)\s+(\S+),\s*(-?\S+)\((\S+)\)'), Store),
    }

    def __init__(self, originalCode: Code, lineno: int) -> None:
        self.originalCodeObj = originalCode
        self.processedCodeList = list()
        self.lineno = lineno
        self.address = 0

    def optimize(self, assumedAddress: int, assumedTagDict: dict, actualAddress: int) -> dict:
        try:
            # optimize
            self.processedCodeList = self.originalCodeObj.optimize(assumedAddress, assumedTagDict)
            # return readdressing info
            self.address = actualAddress
            readdressingInfo = dict()
            for offset, code in enumerate(self.processedCodeList):
                if code.isTag():
                    readdressingInfo[code.name] = self.address + offset * 4
            return readdressingInfo
        except RuntimeError as e:
            raise RuntimeError(f'{self.originalCodeObj}: {e} at line {self.lineno} when optimizing.')
    
    def finalize(self, tags: dict) -> None:
        try:
            tempCodeList = list()
            for offset, code in enumerate(self.processedCodeList):
                tempCodeList.extend(code.finalize(self.address + offset * 4, tags))
            self.processedCodeList = tempCodeList
        except RuntimeError as e:
            raise RuntimeError(f'{self.originalCodeObj}: {e} at line {self.lineno} when finalizing.')

    def encode(self) -> list:
        try:
            tempCodeList = list()
            for code in self.processedCodeList:
                tempCodeList.extend(code.encode())
            return tempCodeList
        except RuntimeError as e:
            raise RuntimeError(f'{self.originalCodeObj}: {e} at line {self.lineno} when encoding.')

    @property
    def assumedLength(self) -> int:
        # in bytes
        return 4 * self.originalCodeObj.assumedLength

    @property
    def actualLength(self) -> int:
        # in bytes
        # should be called after 'optimize'
        # otherwise 'actualLength' will not be updated
        return 4 * self.originalCodeObj.actualLength

    def isDirec(self) -> bool:
        return self.originalCodeObj.isDirec()
    
    def isStartDirec(self) -> bool:
        return self.isDirec() and self.originalCodeObj.isStartDirec()
    
    def isDataDirec(self) -> bool:
        return self.isDirec() and self.originalCodeObj.isDataDirec()
    
    def isCodeDirec(self) -> bool:
        return self.isDirec() and self.originalCodeObj.isCodeDirec()

    def getDirecInfo(self):
        try:
            assert self.isDirec()
            assert not self.isDataDirec()
            assert not self.isCodeDirec()
            return self.originalCodeObj.tag if self.isStartDirec() else self.originalCodeObj.data
        except AssertionError:
            raise RuntimeError(f'{self.originalCodeObj}: invalid operation for non-data directive at line {self.lineno}.')

    def isTag(self) -> bool:
        return self.originalCodeObj.isTag()
    
    def getTagName(self) -> str:
        try:
            assert self.isTag()
            return self.originalCodeObj.name
        except AssertionError:
            raise RuntimeError(f'{self.originalCodeObj}: invalid operation for a non-tag object at {self.lineno}.')

    def __str__(self) -> str:
        # should be called after 'finalize'
        # otherwise 'address' will not be updated
        header = f'{self.lineno:<5} {str(self.originalCodeObj):20s}'
        spacer = ' ' * len(header)
        return '\n'.join(f'{spacer if offset else header} -> {self.address + offset * 4:#08X} {codeObj}' for offset, codeObj in enumerate(self.processedCodeList))

    @staticmethod
    def decode(codeText: str, lineno: int, forSim: bool):
        for _, (decoder, packer) in Block.decoders.items():
            if res := decoder.match(codeText):
                return Block(packer(res.groups(), forSim), lineno)
        else:
            raise RuntimeError(f'unrecognizable instruction \'{codeText}\' at line {lineno} when decoding.')
