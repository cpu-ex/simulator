VPATH = src gui
srcs = $(shell find . -type f -name "*.c")
objs = $(patsubst %.c, %.o, $(srcs))

sim: $(objs)
	gcc -o $@ $^ -lncurses

$(objs): types.h
core.o: mem.h
instr.o, win.o: core.h

.PHONY: clean
clean:
	rm sim $(objs)
