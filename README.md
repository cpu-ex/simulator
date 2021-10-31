# risc-v simulator

## 1. update log

- [2021/10/08] first commit, main framework done, simple sample code in `main.c`,not fully tested.
- [2021/10/10] GUI almost done, somehow functional.
- [2021/10/24] support standard risc-v memory structure + disasm.
- [2021/10/24] complete simple assembler without supporting directives and pseudo instructions
- [2021/10/27] change the output code format from text file to binary, all related parts adapted
- [2021/10/30] support some directives and pseudo instructions for assembler
- [2021/10/31] implement instruction analysis

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

		> relative path to `.s` file needed
	
	- step4: check outputs in `./bin`

		> `xxd fileName.code` or `hexdump fileName.code` would be helpful
	
- dis-assembler
	- step1: `make disasm`
	- step2: `./disasm fileName`

		> binary code with same file name supposed to be in `./bin`
		>
		> using stdout as output

- simulator
	- step1: `make sim`
	- step2: `./sim fileName`

		> binary code with same file name supposed to be in `./bin`
	
	- step3: type `h` for help
	- step4: type `quit` to exit simulator
	- step5: `make clean`
