# assembler
# pseudo code not supported

import os, argparse, struct
import decoder, encoder

class ASM(object):

    DEFAULT_PC = 0x10000
    CODE_SECTION = True
    DATA_SECTION = False

    def __init__(self) -> None:
        self.__decoders = decoder.decoder
        self.__encoders = encoder.encoder

        self.fileName = None
        self.code = list()
        self.codeTag = dict()
        self.data = list()
        self.dataTag = dict()

        self.startTag = None
        self.section = ASM.CODE_SECTION
        self.codeCounter = ASM.DEFAULT_PC + 0x8
        self.dataCounter = 0
    
    def __decode(self, instr: str) -> tuple:
        for name, decoder in self.__decoders.items():
            if res := decoder.match(instr):
                if name.startswith('TAG') or name.startswith('DIREC'):
                    return (name, *res.groups())
                else:
                    _, *info = res.groups()
                    return (name, *info)
        else:
            raise RuntimeError(f'unrecognizable instruction : {instr}')
    
    def __encode(self, instr: tuple, addr: int) -> str:
        # print(f'encoding {instr}\t{hex(addr)}')
        if encoder := self.__encoders.get(instr[0], None):
            return encoder(instr, addr, {**self.codeTag, **self.dataTag})
        else:
            # not suppose to be here
            raise RuntimeError(f'no matched encoder for current instruction : {instr}')
    
    @staticmethod
    def pack2word(data: list, bitLen: int) -> list:
        words = list()
        size = 4 // bitLen
        for i in range(len(data) // size + 1):
            val = 0
            for d in reversed(data[i * size: (i + 1) * size]):
                val <<= bitLen * 8
                val |= d
            words.append(val & 0xFFFFFFFF)
        return words

    def load(self, fileName: str) -> None:
        with open(fileName) as file:
            self.fileName = fileName.split('.')[0]
            # remove all unnecessary parts like space\n\t and comments
            raw = [line.lstrip().rstrip().split('#')[0] for line in file]
        # remove empty lines and seprate tags and instructions
        for line in raw:
            # skip empty lines
            if not line: continue
            # decode
            instr = self.__decode(line)
            name, *info = instr
            if name.startswith('TAG'):
                if self.section == ASM.CODE_SECTION:
                    self.codeTag[info[0]] = self.codeCounter
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
                    data = ASM.pack2word([int(x) & 0xFF for x in info[1].split(',')], 1)
                    self.data += data
                    self.dataCounter += len(data) * 4
                elif name == 'half':
                    data = ASM.pack2word([int(x) & 0xFFFF for x in info[1].split(',')], 2)
                    self.data += data
                    self.dataCounter += len(data) * 4
                elif name == 'word':
                    data = [int(x) & 0xFFFFFFFF for x in info[1].split(',')]
                    self.data += data
                    self.dataCounter += len(data) * 4
                else:
                    raise RuntimeError(f'unsupported directive {name}')
            else:
                self.code.append(instr)
                self.codeCounter += 8 if (name.endswith('LI') or name.endswith('LA')) else 4

    def __outputText(self) -> None:
        # instruction only
        with open(f'../bin/{self.fileName}.txt', 'w') as file:
            for mc in self.code:
                file.write(f'{bin(mc)[2:].zfill(32)}\n')
    
    def __outputBinary(self) -> None:
        # output instruction
        with open(f'../bin/{self.fileName}.code', 'wb') as file:
            for mc in self.code:
                file.write(struct.pack('<I', mc))
        # output data
        with open(f'../bin/{self.fileName}.data', 'wb') as file:
            for d in self.data:
                file.write(struct.pack('<I', d))
    
    def save(self, binary: bool, text: bool) -> None:
        # direct to start point
        if startAddr := self.codeTag.get(self.startTag, None):
            hi, lo = encoder.getHiLo(startAddr - ASM.DEFAULT_PC)
            self.code = [  # jump to start point
                ('AUIPC', 't0', str(hi)),
                ('JALR', 'zero', str(lo), 't0')
            ] + self.code
        else:
            raise RuntimeError(f'no starting tag {self.startTag}')
        # remap data tags
        for tag, addr in self.dataTag.items():
            self.dataTag[tag] = addr + self.codeCounter
        # encode
        bc = list() # binary codes
        addr = ASM.DEFAULT_PC
        for instr in self.code:
            mc = self.__encode(instr, addr) # machine codes
            bc += mc
            addr += len(mc) * 4
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
    args = parser.parse_args()
    # decode + encode + output
    try:
        asm = ASM()
        asm.load(args.fileName)
        asm.save(args.binary, args.text)
    except RuntimeError as e:
        print(e)
