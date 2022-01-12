# assembler

import os, argparse
from decoder import *

class ASM(object):

    DEFAULT_PC = 0x100
    DEFAULT_SP = 0x03FFFFF0
    DATA_SECTION = 0
    CODE_SECTION = 1

    def __init__(self, forSim: bool=True) -> None:
        self.forSim = forSim

        self.fileName = None
        self.rawText = None
        self.code = list()
        self.codeTag = dict()
        self.data = list()
        self.dataTag = dict()
        self.machineCode = list()

        self.startTag = None
        self.section = ASM.CODE_SECTION
        self.codeCounter = ASM.DEFAULT_PC
        self.dataCounter = 0
    
    def load(self, fileName: str) -> None:
        try:
            codeText = open(fileName)
            self.fileName = fileName.split('/')[-1].split('.')[0]
            # remove all unnecessary parts like space\n\t and comments
            self.rawText = [line.lstrip().rstrip().split('#')[0] for line in codeText]
        except FileNotFoundError:
            raise RuntimeError(f'invalid file name: {fileName}.')

    def decode(self) -> None:
        # 3 prepend pseudo instr * 2 assumed length * 4 bytes each
        self.codeCounter += 24
        # remove empty lines and seprate tags and instructions
        for lineno, line in enumerate(self.rawText):
            # skip empty lines
            if not line: continue
            # decode
            codeObj = Block.decode(line, lineno, self.forSim)
            if codeObj.isDirec():
                if codeObj.isStartDirec():
                    self.startTag = codeObj.getDirecInfo()
                elif codeObj.isDataDirec():
                    self.section = ASM.DATA_SECTION
                elif codeObj.isCodeDirec():
                    self.section = ASM.CODE_SECTION
                else:
                    self.data.extend(codeObj.getDirecInfo())
                    self.dataCounter += codeObj.assumedLength
            elif codeObj.isTag():
                if self.section == ASM.CODE_SECTION:
                    self.codeTag[codeObj.getTagName()] = self.codeCounter
                    self.code.append(codeObj)
                else:
                    self.dataTag[codeObj.getTagName()] = self.dataCounter
            else:
                self.code.append(codeObj)
                self.codeCounter += codeObj.assumedLength
        # prepend initialization codes
        if _ := self.codeTag.get(self.startTag, None):
            self.code = [
                Block.decode(f'li sp, {ASM.DEFAULT_SP}', 0, self.forSim),
                Block.decode(f'li hp, {len(self.data) * 4}', 0, self.forSim),
                Block.decode(f'tail {self.startTag}', 0, self.forSim)
            ] + self.code
        else:
            raise RuntimeError(f'no starting tag defined.')

    def optimize(self) -> None:
        assumedAddress = ASM.DEFAULT_PC
        actualAddress = ASM.DEFAULT_PC
        assumedTagDict = {**self.codeTag, **self.dataTag}
        for codeObj in self.code:
            # optimize
            readdressingInfo = codeObj.optimize(assumedAddress, assumedTagDict, actualAddress)
            assumedAddress += codeObj.assumedLength
            # readdress (only code-tag left in this stage)
            self.codeTag.update(readdressingInfo)
            actualAddress += codeObj.actualLength
    
    def finalize(self) -> None:
        # remap data tags (disused)
        # self.dataTag = {tag: (addr + len(self.code) * 4) for tag, addr in self.dataTag.items()}
        # finalize all codes
        for codeObj in self.code:
            codeObj.finalize({**self.codeTag, **self.dataTag})
    
    def encode(self) -> None:
        self.machineCode.clear()
        for codeObj in self.code:
            self.machineCode.extend(codeObj.encode())
    
    def save(self, binaryOutput: bool, textOutput: bool) -> None:
        # prepare for output
        if not os.path.exists('../bin/'):
            os.mkdir('../bin/')
        # output binary file
        if binaryOutput:
            with open(f'../bin/{self.fileName}.code', 'wb') as file:
                for mc in self.machineCode:
                    file.write(struct.pack('>I', mc))
            with open(f'../bin/{self.fileName}.data', 'wb') as file:
                for d in self.data:
                    file.write(struct.pack('>I', d))
        # output text file
        if textOutput:
            with open(f'../bin/{self.fileName}.code.txt', 'w') as file:
                for mc in self.machineCode:
                    file.write(f'{bin(mc)[2:].zfill(32)}\n')
            with open(f'../bin/{self.fileName}.data.txt', 'w') as file:
                for d in self.data:
                    file.write(f'{bin(d)[2:].zfill(32)}\n')
        # warn
        if not (binaryOutput or textOutput):
            print(f'none of the output format specified, check \'python3 asm.py --help\' for detailed information.')

    def printTagInfo(self) -> None:
        print('\n'.join(f'{tag=}\taddr={addr:#08X}' for tag, addr in {**self.codeTag, **self.dataTag}.items()))
    
    def printDetailedInfo(self) -> None:
        print('\n'.join(str(codeObj) for codeObj in self.code))

if __name__ == '__main__':
    # parse arguments
    parser = argparse.ArgumentParser(description='assembler for risc-v')
    parser.add_argument('fileName', help='relative path to .s file needed')
    parser.add_argument('-b', '--binary', action='store_true', required=False, help='export binary file')
    parser.add_argument('-t', '--text', action='store_true', required=False, help='export text file')
    parser.add_argument('--fpga', action='store_false', required=False, help='output codes for fpga, default to false (codes for simulator)')
    parser.add_argument('--tags', action='store_true', required=False, help='print out tags')
    parser.add_argument('-v', '--verbose', action='store_true', required=False, help='print out detailed information')
    args = parser.parse_args()
    # process asm code
    try:
        asm = ASM(forSim=args.fpga)
        asm.load(args.fileName)
        asm.decode()
        asm.optimize()
        asm.finalize()
        asm.encode()
        asm.save(args.binary, args.text)

        if args.tags: asm.printTagInfo()
        if args.verbose: asm.printDetailedInfo()
    except RuntimeError as e:
        print(e)
