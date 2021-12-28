# assembler

import os, argparse, struct
import decoder, encoder

class ASM(object):

    DEFAULT_PC = 0x100
    DEFAULT_SP = 0x03FFFFF0
    CODE_SECTION = True
    DATA_SECTION = False

    def __init__(self, forSim: bool=True) -> None:
        self.__decoders = decoder.getDecoder()
        self.__optimizers = encoder.getOptimizer()
        self.__encoders = encoder.getEncoder(forSim)

        self.fileName = None
        self.code = list()
        self.codeTag = dict()
        self.data = list()
        self.dataTag = dict()

        self.startTag = None
        self.section = ASM.CODE_SECTION
        self.codeCounter = ASM.DEFAULT_PC + 24
        self.dataCounter = 0
    
    def __decode(self, instr: str) -> tuple:
        for name, (decoder, length) in self.__decoders.items():
            if res := decoder.match(instr):
                if name.startswith('TAG') or name.startswith('DIREC'):
                    return (name, *res.groups()), length
                else:
                    _, *info = res.groups()
                    return (name, *info), length
        else:
            raise RuntimeError(f'unrecognizable instruction \'{instr}\'')
    
    def __optimize(self, instr: tuple, length: int, addr: int) -> list:
        if optimizer := self.__optimizers.get(instr[0], None):
            return optimizer(instr, addr, {**self.codeTag, **self.dataTag})
        else:
            return [(instr, length)]

    def __encode(self, instr: tuple, addr: int) -> str:
        # print(f'encoding {instr}\t{hex(addr)}')
        if encoder := self.__encoders.get(instr[0], None):
            return encoder(instr, addr, {**self.codeTag, **self.dataTag})
        else:
            # not suppose to be here
            raise RuntimeError(f'no matched encoder for current instruction \'{instr}\'')
    
    @staticmethod
    def pack2word(data: list, bitLen: int) -> list:
        words = list()
        size = 4 // bitLen
        data += [0] * ((len(data) // size + 1) * size - len(data))
        for i in range(len(data) // size):
            val = 0
            for d in data[i * size: (i + 1) * size]:
                val <<= bitLen * 8
                val |= d
            words.append(val & 0xFFFFFFFF)
        return words

    def load(self, fileName: str) -> None:
        with open(fileName) as file:
            self.fileName = fileName.split('/')[-1].split('.')[0]
            # remove all unnecessary parts like space\n\t and comments
            raw = [line.lstrip().rstrip().split('#')[0] for line in file]
        # remove empty lines and seprate tags and instructions
        lineno = 0
        for line in raw:
            lineno += 1
            # skip empty lines
            if not line: continue
            # decode
            try:
                instr, length = self.__decode(line)
                name, *info = instr
                instr = (instr, length, lineno)  # append line number
            except RuntimeError as e:
                raise RuntimeError(f'{e} at line {lineno}.')
            if name.startswith('TAG'):
                if self.section == ASM.CODE_SECTION:
                    self.codeTag[info[0]] = self.codeCounter
                    self.code.append(((name, *info), 0, lineno))
                else:
                    self.dataTag[info[0]] = self.dataCounter
            elif name.startswith('DIREC'):
                name = info[0]
                if name == 'globl':
                    self.startTag = info[1]
                elif name == 'data':
                    self.section = ASM.DATA_SECTION
                elif name == 'text':
                    self.section = ASM.CODE_SECTION
                elif name == 'byte':
                    data = ASM.pack2word([encoder.imm2int(x) & 0xFF for x in info[1].split(',')], 1)
                    self.data += data
                    self.dataCounter += len(data) * 4
                elif name == 'half':
                    data = ASM.pack2word([encoder.imm2int(x) & 0xFFFF for x in info[1].split(',')], 2)
                    self.data += data
                    self.dataCounter += len(data) * 4
                elif name == 'word':
                    data = [encoder.imm2int(x) & 0xFFFFFFFF for x in info[1].split(',')]
                    self.data += data
                    self.dataCounter += len(data) * 4
                elif name == 'float':
                    data = [encoder.fimm2int(x) & 0xFFFFFFFF for x in info[1].split(',')]
                    self.data += data
                    self.dataCounter += len(data) * 4
                else:
                    raise RuntimeError(f'unsupported directive \'{name}\' at line {lineno}.')
            else:
                self.code.append(instr)
                self.codeCounter += 4 * length

    def optimize(self) -> None:
        optimizedCode = list()
        addr = ASM.DEFAULT_PC
        for instr, length, lineno in self.code:
            try:
                optimized = self.__optimize(instr, length, addr)
                optimizedCode += [(*code, lineno) for code in optimized]
                addr += length * 4
            except RuntimeError as e:
                raise RuntimeError(f'{e} at line {lineno}')
        # readdress
        self.codeTag.clear()
        self.code.clear()
        self.codeCounter = ASM.DEFAULT_PC + 24
        for instr, length, lineno in optimizedCode:
            name, *info = instr
            instr = (instr, length, lineno)
            if name.startswith('TAG'): # only code-tag left
                self.codeTag[info[0]] = self.codeCounter
            self.code.append(instr)
            self.codeCounter += 4 * length

    def __outputText(self) -> None:
        # output instruction
        with open(f'../bin/{self.fileName}.code.txt', 'w') as file:
            for mc in self.code:
                file.write(f'{bin(mc)[2:].zfill(32)}\n')
        # output data
        with open(f'../bin/{self.fileName}.data.txt', 'w') as file:
            for d in self.data:
                file.write(f'{bin(d)[2:].zfill(32)}\n')
    
    def __outputBinary(self) -> None:
        # output instruction
        with open(f'../bin/{self.fileName}.code', 'wb') as file:
            for mc in self.code:
                file.write(struct.pack('>I', mc))
        # output data
        with open(f'../bin/{self.fileName}.data', 'wb') as file:
            for d in self.data:
                file.write(struct.pack('>I', d))
    
    def save(self, binary: bool, text: bool) -> None:
        # direct to start point
        if startAddr := self.codeTag.get(self.startTag, None):
            hiOfSp, loOfSp = encoder.getHiLo(ASM.DEFAULT_SP)
            hiOfHp, loOfHp = encoder.getHiLo(len(self.data) * 4)
            hiOfStart, loOfStart = encoder.getHiLo(startAddr - ASM.DEFAULT_PC - 16)
            self.code = [
                # preset sp to DEFAULT_SP
                (('LUI', 'sp', str(hiOfSp)), 1, -6),
                (('ADDI', 'sp', 'sp', str(loOfSp)), 1, -5),
                # preset hp right after static data
                (('LUI', 'hp', str(hiOfHp)), 1, -4),
                (('ADDI', 'hp', 'hp', str(loOfHp)), 1, -3),
                # jump to start point
                (('AUIPC', 't0', str(hiOfStart)), 1, -2),
                (('JALR', 'zero', str(loOfStart), 't0'), 1, -1)
            ] + self.code
        else:
            raise RuntimeError(f'no starting tag defined.')
        # remap data tags (disused)
        # for tag, addr in self.dataTag.items():
        #     self.dataTag[tag] = addr + self.codeCounter
        # encode
        bc = list() # binary codes
        addr = ASM.DEFAULT_PC
        for instr, _, lineno in self.code:
            try:
                mc = self.__encode(instr, addr) # machine codes
                bc += mc
                addr += len(mc) * 4
            except RuntimeError as e:
                raise RuntimeError(f'{e} at line {lineno}.')
        self.code = bc
        # prepare for output
        if not os.path.exists('../bin/'):
            os.mkdir('../bin/')
        # output binary file
        if binary or (not (binary ^ text)):
            self.__outputBinary()
        # output text file
        if text or (not (binary ^ text)):
            self.__outputText()

if __name__ == '__main__':
    # parse arguments
    parser = argparse.ArgumentParser(description='assembler for risc-v')
    parser.add_argument('fileName', help='relative path to .s file needed')
    parser.add_argument('-b', '--binary', action='store_true', required=False, help='export binary file')
    parser.add_argument('-t', '--text', action='store_true', required=False, help='export text file')
    parser.add_argument('--fpga', action='store_false', required=False, help='output codes for fpga, default to false (codes for simulator)')
    parser.add_argument('--tags', action='store_true', required=False, help='print out tags')
    args = parser.parse_args()
    # decode + encode + output
    try:
        asm = ASM(forSim=args.fpga)
        asm.load(args.fileName)
        asm.optimize()

        if args.tags:
            for tag, addr in {**asm.codeTag, **asm.dataTag}.items():
                print(f'{tag=}\taddr={hex(addr)}')
        
        asm.save(args.binary, args.text)
    except RuntimeError as e:
        print(e)
