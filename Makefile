VPATH = src gui asm
srcs = $(wildcard *.c) $(wildcard src/*.c) $(wildcard gui/*.c)
objs = $(patsubst %.c, %.o, $(srcs))

all: sim disasm

sim: $(objs)
	gcc -o $@ $^ -lncurses -lm

disasm: ./asm/disasm.o ./src/instr.o
	gcc -o $@ $^

$(objs): types.h
mmu.o: mem.h cache.h
core.o: mmu.h
exec.o, win.o: core.h
exec.o: instr.h
disasm.o: types.h instr.h
gui.o: splashWin.h helpWin.h mainWin.h analysisWin.h

.PHONY: clean
clean:
	rm sim $(objs) disasm ./asm/disasm.o
