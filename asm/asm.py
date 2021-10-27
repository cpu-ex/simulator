# assembler
# pseudo code not supported

import os, argparse
import decoder, encoder

DEFAULT_PC = 0x10000

class ASM(object):

    def __init__(self, *opts) -> None:
        self.__decoders = decoder.decoder
        self.__encoders = encoder.encoder

        self.fileName = None
        self.source = list()
        self.tags = dict()
        self.code = list()

        self.verbose = opts[0]
    
    def __decode(self, instr: str) -> tuple:
        for name, decoder in self.__decoders.items():
            if res := decoder.match(instr):
                if name == 'TAG' or name == 'DIREC':
                    return (name, *res.groups())
                else:
                    name, *info = res.groups()
                    return (name.upper(), *info)
        else:
            raise RuntimeError(f'unrecognizable instruction : {instr}')
    
    def __encode(self, instr: tuple, addr: int) -> str:
        if self.verbose:
            print(f'encoding {instr}\t{hex(addr)}')
        if encoder := self.__encoders.get(instr[0], None):
            return encoder(instr, addr, self.tags)
        else:
            # not suppose to be here
            raise RuntimeError(f'no matched encoder for current instruction : {instr}')
    
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
            instr = self.__decode(line)
            name, *info = instr
            if name == 'TAG':
                self.tags[info[0]] = addr
            elif name == 'DIREC':
                pass  # skip this currently
            else:
                self.source.append(instr)
                addr += 4

    def __outputText(self) -> None:
        # instruction only
        with open(f'../bin/{self.fileName}.txt', 'w') as file:
            for mc in self.code:
                file.write(f'{bin(mc)[2:].zfill(32)}\n')
    
    def __outputBinary(self) -> None:
        # output instruction
        with open(f'../bin/{self.fileName}.code', 'wb') as file:
            for mc in self.code:
                file.write(mc.to_bytes(4, 'little'))
        # output data TODO
    
    def save(self, binary: bool, text: bool) -> None:
        # encode
        for idx, instr, in enumerate(self.source):
            addr = DEFAULT_PC + 4 * idx
            self.code.append(self.__encode(instr, addr))
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
    parser.add_argument('-v', '--verbose', action='store_true', required=False)
    args = parser.parse_args()
    # decode + encode + output
    asm = ASM(args.verbose)
    asm.load(args.fileName)
    asm.save(args.binary, args.text)
