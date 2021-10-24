VPATH = src gui
srcs = $(shell find . -type f -name "*.c")
objs = $(patsubst %.c, %.o, $(srcs))

sim: $(objs)
	gcc -o $@ $^ -lncurses

$(objs): types.h
core.o: mem.h
exec.o, win.o: core.h
exec.o: instr.h

.PHONY: clean
clean:
	rm sim $(objs)
