# assembler
# pseudo code not supported

import os, sys
import decoder, encoder

DEFAULT_PC = 0x10000

class ASM(object):

    def __init__(self) -> None:
        self.__decoders = decoder.decoder
        self.__encoders = encoder.encoder

        self.fileName = None
        self.source = list()
        self.tags = dict()
    
    def decode(self, instr: str) -> tuple:
        for name, decoder in self.__decoders.items():
            if res := decoder.match(instr):
                if name == 'TAG' or name == 'DIREC':
                    return (name, *res.groups())
                else:
                    name, *info = res.groups()
                    return (name.upper(), *info)
        else:
            pass #TODO: raise exception
    
    def encode(self, instr: tuple, addr: int) -> str:
        print(f'encoding {instr}\t{hex(addr)}')
        if encoder := self.__encoders.get(instr[0], None):
            return encoder(instr, addr, self.tags)
        else:
            pass #TODO: raise exception
    
    def load(self, fileName: str) -> None:
        with open(fileName) as file:
            self.fileName = fileName.split('.')[0]
            # remove all unnecessary parts like space\n\t and comments
            raw = [line.lstrip().rstrip().split('#')[0] for line in file]
        # remove empty lines and seprate tags and instructions
        addr = DEFAULT_PC
        for line in raw:
            # skip empty lines
            if not line: continue
            # decode
            instr = self.decode(line)
            name, *info = instr
            if name == 'TAG':
                self.tags[info[0]] = addr
            elif name == 'DIREC':
                pass  # skip this currently
            else:
                self.source.append(instr)
                addr += 4

    def output(self):
        if not os.path.exists('../bin/'):
            os.mkdir('../bin/')
        buffer = list()
        for idx, instr in enumerate(self.source):
            addr = DEFAULT_PC + 4 * idx
            buffer.append(self.encode(instr, addr))
        with open(f'../bin/{self.fileName}.code', 'w') as file:
            file.writelines('\n'.join(buffer))

if __name__ == '__main__':
    asm = ASM()
    asm.load(sys.argv[1])
    asm.output()
    # tests
    # print(asm.decode('main:'))
    # print(asm.decode('.align 2'))
    # print(asm.decode('lui a0, 100'))
    # print(asm.decode('jal a0, loop'))
    # print(asm.decode('beq s0, s1, 100'))
    # print(asm.decode('lb t0, -8(a0)'))
    # print(asm.decode('addi t0, t1, -100'))
    # print(asm.decode('add s0, s1, s2'))
