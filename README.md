# risc-v simulator

## 1. Update log

- [2021/10/08] first commit, main framework done, simple sample code in `main.c`,not fully tested.
- [2021/10/10] GUI almost done, somehow functional.
- [2021/10/24] implement standard risc-v memory structure + disasm.
- [2021/10/24] complete simple assembler without supporting directives and pseudo instructions
- [2021/10/27] change the output code format from text file to binary, all related parts adapted
- [2021/10/30] support some directives and pseudo instructions for assembler
- [2021/10/31] implement instruction analysis
- [2021/11/09] support RV32M and RV32F
- [2021/11/12] have every related parts customized
- [2021/11/19] implement a cache system
- [2021/11/21] visualize the cache structure
- [2021/11/29] implement a branch predictor
- [2021/12/01] improve the interaction of GUI
- [2021/12/05] introduce stall counter and display executing time prediction to analysis window
- [2021/12/12] implement automatic stepping with window updated
- [2021/12/14] implement information dump for simulator
- [2021/12/21] implement uart output for simulator
- [2021/12/24] implement uart input for simulator
- [2021/12/28] fix the exception of branching too far for assembler
- [2022/01/08] refactor assembler
- [2022/01/13] adjust some parameters for executing time prediction
- [2022/01/15] finalize LITE mode
- [2022/01/18] support switching LITE mode and NOCACHE mode as optional argument

## 2. How to use

***recommended workflow as well***

### 2.1 Prerequisites

- gcc (for simulator & disasm)
- ncurses (for simulator)

	> MacOS: supported
	> 
	> Ubuntu: run `sudo apt-get install libncurses5-dev`

	> simulator looks better in brighter colors and here is a setting I personally prefer
	> 
	> | color   | hex code | sample                                                                                 |
	> | :------ | :------- | :------------------------------------------------------------------------------------- |
	> | black   | #1C2126  | <img src='https://www.thecolorapi.com/id?format=svg&named=false&hex=1C2126' height=48> |
	> | red     | #E06C75  | <img src='https://www.thecolorapi.com/id?format=svg&named=false&hex=E06C75' height=48> |
	> | green   | #98C379  | <img src='https://www.thecolorapi.com/id?format=svg&named=false&hex=98C379' height=48> |
	> | yellow  | #E5C07B  | <img src='https://www.thecolorapi.com/id?format=svg&named=false&hex=E5C07B' height=48> |
	> | blue    | #61AFEF  | <img src='https://www.thecolorapi.com/id?format=svg&named=false&hex=61AFEF' height=48> |
	> | magenta | #C678DD  | <img src='https://www.thecolorapi.com/id?format=svg&named=false&hex=C678DD' height=48> |
	> | cyan    | #56B6C2  | <img src='https://www.thecolorapi.com/id?format=svg&named=false&hex=56B6C2' height=48> |
	> | white   | #DCDFE4  | <img src='https://www.thecolorapi.com/id?format=svg&named=false&hex=DCDFE4' height=48> |

- python3.8 (for assembler)

### 2.2 Assembler

> supported instructions
> 
> - RV32I: *TODO*
> 
> - RV32F: *TODO*
> 
> - Pseudo instructions: *TODO*

- step1: `cd ./asm`
- step2: check `python3 asm.py -h` for help
- step3: do some executions like `python3 asm.py minrt16.s -b`
- step4: check outputs in `./bin`

	> `xxd fileName.code` or `hexdump fileName.code` would be helpful
	
### 2.3 Dis-assembler

- step1: `make disasm`
- step2: `./disasm fileName`

	> supposing the existence of `./bin/fileName.code`
	>
	> using stdout as output

### 2.4 Simulator

> customizable settings
> 
> - Cache (under `src/cache.h`)
> 	- block size
> 	- associativity
>	- block switching policy (specially optimized for LRU)
> - Branch Predictor (under `src/branch_predictor.h`)
> 	- prediction policy
> 	- size of PHT
> 
> run `make clean; make` to apply changes

- step1: `make sim`
- step2: check `./sim -h` for help
- step3: do some executions like `./sim minrt16 --sld bin/contest.sld`
- step4: type `help` to get more information about gui mode
- step5: type `quit` to exit simulator
- step6: `make clean`

## 3. Executing time prediction

### 3.1 No cache ver

- time & hit rate
	- clk = 10Mhz, PHT size = 1024
	- error of always untaken = 0.24% ~ 0.49%

	| branch prediction policy | minrt16           | minrt64            | minrt128           | minrt512            |
	| :----------------------- | :---------------- | :----------------- | :----------------- | :------------------ |
	| always untaken           | 111.268 (46.237%) | 1087.053 (46.716%) | 3333.707 (46.652%) | not tested          |
	| always taken             | 111.160 (53.763%) | 1086.138 (53.284%) | 3330.855 (53.348%) | not tested          |
	| 2bit counter             | 111.118 (56.741%) | 1085.694 (56.478%) | 3329.513 (56.498%) | 37721.116 (56.594%) |
	| bimodal                  | 110.874 (73.790%) | 1083.313 (73.579%) | 3322.219 (73.619%) | not tested          |
	| Gshare                   | 110.654 (89.182%) | 1081.161 (89.043%) | 3315.675 (88.978%) | not tested          |

## 4. Supported instructions

## 4.1 Instruction format

<img src='https://devopedia.org/images/article/110/3808.1535301636.png' height=200>

## 4.2 RV32I

| name                        | assembly             | fmt | opcode  | funct3 | funct7         | description                       |
| :-------------------------- | :------------------- | :-- | :------ | :----- | :------------- | :-------------------------------- |
| **ARITH**                   |                      |     |         |        |                |                                   |
| add                         | `add rd, rs1, rs2`   | R   | 0110011 | 000    | 0000000        | rd = rs1 + rs2                    |
| sub                         | `sub rd, rs1, rs2`   | R   | 0110011 | 000    | 0100000        | rd = rs1 - rs2                    |
| shift left logical          | `sll rd, rs1, rs2`   | R   | 0110011 | 001    | 0000000        | rd = rs1 << rs2                   |
| set less than               | `slt rd, rs1, rs2`   | R   | 0110011 | 010    | 0000000        | rd = (rs1 < rs2) ? 1 : 0          |
| set less than (U)           | `sltu rd, rs1, rs2`  | R   | 0110011 | 011    | 0000000        | rd = (rs1 < rs2) ? 1 : 0          |
| xor                         | `xor rd, rs1, rs2`   | R   | 0110011 | 100    | 0000000        | rd = rs1 ^ rs2                    |
| shift right logical         | `srl rd, rs1, rs2`   | R   | 0110011 | 101    | 0000000        | rd = rs1 >> rs2 (zero-ext)        |
| shift right arith           | `sra rd, rs1, rs2`   | R   | 0110011 | 101    | 0100000        | rd = rs1 >> rs2 (msb-ext)         |
| or                          | `or rd, rs1, rs2`    | R   | 0110011 | 110    | 0000000        | rd = rs1 | rs2                    |
| and                         | `and rd, rs1, rs2`   | R   | 0110011 | 111    | 0000000        | rd = rs1 & rs2                    |
| **ARITH-I**                 |                      |     |         |        |                |                                   |
| add immediate               | `addi rd, rs1, imm`  | I   | 0010011 | 000    |                | rd = rs1 + sext(imm)              |
| shift left logical imm      | `slli rd, rs1, imm`  | I   | 0010011 | 001    | imm[11:5]=0x00 | rd = rs1 << imm[4:0]              |
| set less than imm           | `slti rd, rs1, imm`  | I   | 0010011 | 010    |                | rd = (rs1 < sext(imm)) ? 1 : 0    |
| set less than imm (U)       | `sltiu rd, rs1, imm` | I   | 0010011 | 011    |                | rd = (rs1 < imm) ? 1 : 0          |
| xor immediate               | `xori rd, rs1, imm`  | I   | 0010011 | 100    |                | rd = rs1 ^ sext(imm)              |
| shift right logical imm     | `srli rd, rs1, imm`  | I   | 0010011 | 101    | imm[11:5]=0x00 | rd = rs1 >> imm[4:0] (zero-ext)   |
| shift right arith imm       | `srai rd, rs1, imm`  | I   | 0010011 | 101    | imm[11:5]=0x20 | rd = rs1 >> imm[4:0] (msb-ext)    |
| or immediate                | `ori rd, rs1, imm`   | I   | 0010011 | 110    |                | rd = rs1 | sext(imm)              |
| and immediate               | `andi rd, rs1, imm`  | I   | 0010011 | 111    |                | rd = rs1 & sext(imm)              |
| **LOAD**                    |                      |     |         |        |                |                                   |
| load word                   | `lw rd, imm(rs1)`    | I   | 0000011 | 010    |                | rd = M[rs1+sext(imm)]             |
| **STORE**                   |                      |     |         |        |                |                                   |
| store word                  | `sw rs2, imm(rs1)`   | S   | 0100011 | 010    |                | M[rs1+sext(imm)] = rs2            |
| **BRANCH**                  |                      |     |         |        |                |                                   |
| branch equal                | `beq rs1, rs2, tag`  | B   | 1100011 | 000    |                | if (rs1 == rs2) PC += (&tag - pc) |
| branch not equal            | `bne rs1, rs2, tag`  | B   | 1100011 | 001    |                | if (rs1 != rs2) PC += (&tag - pc) |
| branch less than            | `blt rs1, rs2, tag`  | B   | 1100011 | 100    |                | if (rs1 < rs2) PC += (&tag - pc)  |
| branch great equal than     | `bge rs1, rs2, tag`  | B   | 1100011 | 101    |                | if (rs1 >= rs2) PC += (&tag - pc) |
| branch less than (U)        | `bltu rs1, rs2, tag` | B   | 1100011 | 110    |                | if (rs1 < rs2) PC += (&tag - pc)  |
| branch great equal than (U) | `bgeu rs1, rs2, tag` | B   | 1100011 | 111    |                | if (rs1 >= rs2) PC += (&tag - pc) |
| **JUMP**                    |                      |     |         |        |                |                                   |
| jump and link               | `jal rd, tag`        | J   | 1101111 |        |                | rd = PC + 4; PC += (&tag - pc)    |
| jump and link register      | `jalr rd, imm(rs1)`  | I   | 0010111 | 000    |                | rd = PC + 4; PC = rs1 + sext(imm) |
| **LUI & AUIPC**             |                      |     |         |        |                |                                   |
| load upper immediate        | `lui rd, imm`        | U   | 0110111 |        |                | rd = imm << 12                    |
| add upper immediate to PC   | `auipc rd, imm`      | U   | 0010111 |        |                | rd = PC + (imm << 12)             |

## 4.3 RV32F

| name                          | assembly                     | fmt | opcode  | funct3  | funct7  | description                             |
| :---------------------------- | :--------------------------- | :-- | :------ | :------ | :------ | :-------------------------------------- |
| **F-LOAD**                    |                              |     |         |         |         |                                         |
| float load word               | `flw frd, imm(rs1)`          | I   | 0000111 | 010     |         | frd = M[rs1+sext(imm)]                  |
| **F-STORE**                   |                              |     |         |         |         |                                         |
| float store word              | `fsw frs2, imm(rs1)`         | S   | 0100111 | 010     |         | M[rs1+sext(imm)] = frs2                 |
| **F-MOVE**                    |                              |     |         |         |         |                                         |
| move float to integer         | `fmv.x.w rd, frs1`           | R   | 1010011 | 000     | 1110000 | rd = \*(int\*)(&frs1)                   |
| move integer to float         | `fmv.w.x frd, rs1`           | R   | 1010011 | 000     | 1111000 | frd = \*(float\*)(&rs1)                 |
| **F-CONVERT**                 |                              |     |         |         |         |                                         |
| convert from signed int       | `fcvt.s.w frd, rs1` (rs2=0)  | R   | 1010011 | ignored | 1101000 | frd = (float)(signed)rs1                |
| convert from unsigned int     | `fcvt.s.wu frd, rs1` (rs2=1) | R   | 1010011 | ignored | 1101000 | frd = (float)(unsigned)rs1              |
| convert to signed int         | `fcvt.w.s rd, frs1` (rs2=0)  | R   | 1010011 | ignored | 1100000 | rd = (signed)frs1                       |
| convert to unsigned int       | `fcvt.wu.s rd, frs1` (rs2=1) | R   | 1010011 | ignored | 1100000 | rd = (unsigned)frs1                     |
| **F-SIGN_INJECT**             |                              |     |         |         |         |                                         |
| float sign injection          | `fsgnj frd, frs1, frs2`      | R   | 1010011 | 000     | 0010000 | frd = {frs2[31], frs1[30:0]}            |
| float sign neg injection      | `fsgnjn frd, frs1, frs2`     | R   | 1010011 | 001     | 0010000 | frd = {~frs2[31], frs1[30:0]}           |
| float sign xor injection      | `fsgnjx frd, frs1, frs2`     | R   | 1010011 | 010     | 0010000 | frd = {frs1[31] ^ frs2[31], frs1[30:0]} |
| **F-ARITH**                   |                              |     |         |         |         |                                         |
| float add                     | `fadd frd, frs1, frs2`       | R   | 1010011 | ignored | 0000000 | frd = frs1 + frs2                       |
| float sub                     | `fsub frd, frs1, frs2`       | R   | 1010011 | ignored | 0000100 | frd = frs1 - frs2                       |
| float multiply                | `fmul frd, frs1, frs2`       | R   | 1010011 | ignored | 0001000 | frd = frs1 * frs2                       |
| float divide                  | `fdiv frd, frs1, frs2`       | R   | 1010011 | ignored | 0001100 | frd = frs1 / frs2                       |
| float square root             | `fsqrt frd, frs1` (frs2=0)   | R   | 1010011 | ignored | 0001100 | frd = sqrt(frs1)                        |
| float compare equal           | `feq rd, frs1, frs2`         | R   | 1010011 | 010     | 1010000 | rd = (frs1 == frs2) ? 1 : 0             |
| float compare less than       | `flt rd, frs1, frs2`         | R   | 1010011 | 001     | 1010000 | rd = (frs1 < frs2) ? 1 : 0              |
| float compare less equal than | `fle rd, frs1, frs2`         | R   | 1010011 | 000     | 1010000 | rd = (frs1 <= frs2) ? 1 : 0             |

## 4.4 Customized instructions

| name                  | assembly              | fmt | opcode  | funct3 | description                                                                 |
| :-------------------- | :-------------------- | :-- | :------ | :----- | :-------------------------------------------------------------------------- |
| **STORE**             |                       |     |         |        |                                                                             |
| store instruction     | `swi rs2, imm(rs1)`   | S   | 0100011 | 011    | Instr[rs1+sext(imm)] = rs2                                                  |
| **ENVIRONMENT**       |                       |     |         |        |                                                                             |
| environment break     | `ebreak imm`          | I   | 1110011 |        | transfer control to debugger at the imm times encountering this instruction |
| (AKA set breakpoint)  |                       |     |         |        | using imm=0 to imply the end of program                                     |
| **VECTOR** (for 2nd)  |                       |     |         |        |                                                                             |
| vector load word      | `vlw v0, imm(rs1)`    | I   | 0000111 | 000    | v0 = M[rs1+sext(imm)][127:0]                                                |
| vector store word     | `vsw v0, imm(rs1)`    | S   | 0100111 | 000    | M[rs1+sext(imm)][127:0] = v0                                                |
| burst load word       | `blw rd, imm(rs1)`    | I   | 0000111 | 001    | rd, rd + 1, rd + 2, rd + 3 = M[rs1+sext(imm)][127:0]                        |
| burst store word      | `bsw rs2, imm(rs1)`   | S   | 0100111 | 001    | M[rs1+sext(imm)][127:0] = {rs2, rs2 + 1, rs2 + 2, rs2 + 3}                  |
| burst load float      | `bflw frd, imm(rs1)`  | I   | 0000111 | 100    | frd, frd + 1, frd + 2, frd + 3 = M[rs1+sext(imm)][127:0]                    |
| burst store float     | `bfsw frs2, imm(rs1)` | S   | 0100111 | 100    | M[rs1+sext(imm)][127:0] = {frs2, frs2 + 1, frs2 + 2, frs2 + 3}              |
| move scalar to vector | `vmv.v.s v0, rs1, rs2, rs3, rs4` | ↓ | 1010111 | ↓ | v0 = {rs1, rs2, rs3, rs4} |
| move vector to scalar | `vmv.s.v rs1, rs2, rs3, rs4, v0` | ↓ | 1010111 | ↓ | {rs1, rs2, rs3, rs4} = v0 |

|           | 31 ~ 26 | 25 ~ 20 | 19 ~ 15  | 14     | 13  | 12 ~ 7 | 6 ~ 0   |
| :-------- | :------ | :------ | :------- | :----- | :-- | :----- | :------ |
| `vmv.v.s` | rs3     | rs2     | rs1[4:0] | rs1[5] | 0   | rs4    | 1010111 |
| `vmv.s.v` | rs3     | rs2     | rs1[4:0] | rs1[5] | 1   | rs4    | 1010111 |

## 4.5 Pseudo instructions

| name                    | assembly         | base instruction                   |
| :---------------------- | :--------------- | :--------------------------------- |
| no operation            | `nop`            | `addi zero, zero, 0`               |
| load immediate          | `li rd, imm`     | `lui rd, %hi(imm)`                 |
|                         |                  | `addi rd, rd, %lo(imm)`            |
| load address            | `la rd, tag`     | `auipc rd, %hi(&tag)`              |
|                         |                  | `addi rd, rd, %lo(&tag)`           |
| one's complement        | `not rd, rs1`    | `xori rd, rs1, -1`                 |
| copy register           | `mv rd, rs1`     | `addi rd, rs1, 0`                  |
| jump                    | `j tag`          | `jal zero, tag`                    |
| jump and link           | `jal tag`        | `jal ra, tag`                      |
| jump and link register  | `jalr rs`        | `jalr ra, 0(rs)`                   |
| call faraway subroutine | `call tag`       | `auipc t1, %hi(&tag - pc)`         |
|                         |                  | `jalr ra, %lo(&tag - pc)(t1)`      |
| tail faraway subroutine | `tail tag`       | `auipc t1, %hi(&tag -pc)`          |
|                         |                  | `jalr zero, %lo(&tag - pc)(t1)`    |
| copy float register     | `fmv frd, frs1`  | `fsgnj frd, frs1, frs1`            |
| absolute value of float | `fabs frd, frs1` | `fsgnjx frd, frs1, frs1`           |
| negative value of float | `fneg frd, frs1` | `fsgnjn frd, frs1, frs1`           |
| environment break       | `ebreak`         | `ebreak 0`                         |

## 4.6 Specially optimized

- `li`
- `branch`
