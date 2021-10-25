# risc-v simulator

## update log

- [2021/10/08] first commit, main framework done, simple sample code in `main.c`,not fully tested.
- [2021/10/10] GUI almost done, somehow functional.
- [2021/10/24] support standard risc-v memory structure + disasm.
- [2021/10/24] complete simple assembler without supporting directives and pseudo instructions

## how to build

- prerequisites
	- gcc
	- ncurses
		> MacOS: supported
		> 
		> Ubuntu: run `sudo apt-get install libncurses5-dev`
	- python3.8

- assembler
	- step1: `cd ./asm`
	- step2: `python3 asm.py fileName.s`
		
		> relative path to `.s` file needed
	- step3: check outputs in `./bin`
	
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
	- step3: `make clean`

