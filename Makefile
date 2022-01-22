VPATH = src gui asm
srcs = $(wildcard *.c) $(wildcard src/*.c) $(wildcard gui/*.c)
objs = $(patsubst %.c, %.o, $(srcs))

all: sim disasm

sim: $(objs)
	gcc -o $@ $^ -lncurses -lm -O3

disasm: ./asm/disasm.o ./src/instr.o
	gcc -o $@ $^

$(objs): global.h
cache.o: mem.h
mmu.o: mem.h cache.h
core.o: mmu.h branch_predictor.h
sim.o: core.h gui.h
disasm.o: global.h instr.h
gui.o: core.h splash_win.h help_win.h main_win.h analysis_win.h cache_win.h

.PHONY: clean
clean:
	rm sim $(objs) disasm ./asm/disasm.o
