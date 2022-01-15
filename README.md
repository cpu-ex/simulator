# risc-v simulator

## 1. update log

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

## 2. how to build

***recommended workflow as well***

- prerequisites
	- gcc
	- ncurses

		> MacOS: supported
		> 
		> Ubuntu: run `sudo apt-get install libncurses5-dev`
	
	- python3.8

- assembler
	- step1: `cd ./asm`
	- step2: check `python3 asm.py -h` for help
	- step3: `python3 asm.py fileName.s`

		> supposing the existence of `./fileName.s` (relative path supported)
	
	- step4: check outputs in `./bin`

		> `xxd fileName.code` or `hexdump fileName.code` would be helpful
	
- dis-assembler
	- step1: `make disasm`
	- step2: `./disasm fileName`

		> supposing the existence of `./bin/fileName.code`
		>
		> using stdout as output

- simulator

	> customizable settings
	> 
	> - Cache (under `src/cache.h`)
	> 	- block size
	> 	- associativity
	>	- block switching policy
	> - Branch Predictor (under `src/branch_predictor.h`)
	> 	- prediction policy
	> 	- size of PHT
	> 
	> run `make clean; make` to apply changes

	- step1: `make sim`
	- step2: `./sim codeFileName [sldFileName]`

		> codeFileName: supposing the existence of `./bin/codeFileName.code`
		> 
		> sldFileName: supposing the optional existence of `./sldFileName.sld` which is a binary file
		> 
		> e.g.: `./sim uart asm/test-codes/uart`
	
	- step3: type `h` for help
	- step4: type `quit` to exit simulator
	- step5: `make clean`

## 3. time prediction

- no cache ver : time (hit rate)

	| branch prediction policy | minrt16           | minrt64            | minrt128           | minrt512            |
	| :----------------------- | :---------------- | :----------------- | :----------------- |:------------------- |
	| always untaken           | 111.268 (46.237%) | 1087.053 (46.716%) | 3333.707 (46.652%) | no tested           |
	| always taken             | 111.160 (53.763%) | 1086.138 (53.284%) | 3330.855 (53.348%) | no tested           |
	| 2bit counter             | 111.118 (56.741%) | 1085.694 (56.478%) | 3329.513 (56.498%) | 37721.116 (56.594%) |
	| bimodal                  | 110.874 (73.790%) | 1083.313 (73.579%) | 3322.219 (73.619%) | no tested           |
	| Gshare                   | 110.654 (89.182%) | 1081.161 (89.043%) | 3315.675 (88.978%) | no tested           |

	- clk = 10Mhz, PHT size = 1024
	- error of always untaken = 0.24% ~ 0.49%

- ...
