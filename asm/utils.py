# utils

import sys, re, struct

reg = {
    'zero': 0,
    'ra': 1,
    'sp': 2,
    'hp': 3, 'gp': 3,
    'tp': 4,
    't0': 5, 't1': 6, 't2': 7,
    's0': 8, 'fp': 8, 's1': 9,
    'a0': 10, 'a1': 11, 'a2': 12, 'a3': 13, 'a4': 14, 'a5': 15, 'a6': 16, 'a7': 17,
    's2': 18, 's3': 19, 's4': 20, 's5': 21, 's6': 22, 's7': 23, 's8': 24, 's9': 25, 's10': 26, 's11': 27,
    't3': 28, 't4': 29, 't5': 30, 't6': 31
}

freg = {
    'ft0': 0, 'ft1': 1, 'ft2': 2, 'ft3': 3, 'ft4': 4, 'ft5': 5, 'ft6': 6, 'ft7': 7,
    'fs0': 8, 'fs1': 9,
    'fa0': 10, 'fa1': 11, 'fa2': 12, 'fa3': 13, 'fa4': 14, 'fa5': 15, 'fa6': 16, 'fa7': 17,
    'fs2': 18, 'fs3': 19, 'fs4': 20, 'fs5': 21, 'fs6': 22, 'fs7': 23, 'fs8': 24, 'fs9': 25, 'fs10': 26, 'fs11': 27,
    'ft8': 28, 'ft9': 29, 'ft10': 30, 'ft11': 31
}

def getHiLo(val: int) -> tuple:
    high = ((((val >> 12) & 0xFFFFF) + (1 if val & 0x800 else 0)) << 12) & 0xFFFFFFFF
    low = (val & 0xFFF) | (0xFFFFF000 if val & 0x800 else 0)
    high = struct.unpack('>i', struct.pack('>I', high))[0]
    low = struct.unpack('>i', struct.pack('>I', low))[0]
    return high, low

def reg2idx(name: str) -> int:
    if (idx := reg.get(name, None)) is not None:
        return idx
    elif (idx := freg.get(name, None)) is not None:
        return idx
    elif res := re.match(r'[xf](\d+)', name):
        idx = int(res.groups()[0])
        if idx < 32:
            return idx
    raise RuntimeError(f'invalid register \'{name}\'')

def idx2reg(idx: int, type: str='x') -> str:
    d = reg if type == 'x' else freg
    names = [key for key, val in d.items() if val == idx]
    return names[0] if names else None

def imm2int(imm: str) -> int:
    try:
        # prevent converting strings like 'a0' to 160
        if imm in reg.keys() or imm in freg.keys():
            raise ValueError
        try:
            return int(imm, base=10)
        except ValueError:
            return int(imm, base=16)
    except ValueError:
        raise RuntimeError(f'invalid digital value \'{imm}\'')

def fimm2int(imm: str) -> int:
    try:
        bytes = struct.pack('>f', float(imm))
        return struct.unpack('>I', bytes)[0]
    except ValueError:
        raise RuntimeError(f'invalid float value \'{imm}\'')

def checkImm(imm: int, bitLen: int, signed: bool) -> None:
    boundaryMin = -(1 << (bitLen - 1)) if signed else 0
    boundaryMax = (1 << (bitLen - 1)) if signed else (1 << bitLen)
    try:
        assert boundaryMin <= imm < boundaryMax
    except AssertionError:
        raise RuntimeError(f'digital value \'{imm}\' out of bound ({boundaryMin} ~ {boundaryMax})')

def tag2offset(name: str, tags: dict, addr: int) -> int:
    if (dst := tags.get(name, None)) is not None:
        return dst - addr
    else:
        raise RuntimeError(f'no tag name \'{name}\'')

def tag2imm(name: str, tags: dict) -> int:
    if (dst := tags.get(name, None)) is not None:
        return dst
    else:
        raise RuntimeError(f'no tag name \'{name}\'')

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

def inc():
    counter = -1
    def inner():
        nonlocal counter
        counter += 1
        return counter
    return inner
inc = inc()

def printProgress(iteration, total, info='', barLength=32):
    percent = f'{100 * (iteration / float(total)):.1f}'
    filledLength = int(round(barLength * iteration / float(total)))
    bar = '#' * filledLength + '-' * (barLength - filledLength)
    sys.stderr.write(f'\r{info:20s} [{bar}] {percent}%')
    if iteration == total:
        sys.stderr.write(u'\u001b[2K')
        sys.stderr.write(u'\u001b[0G')
    sys.stderr.flush()
