srcs = mem.c core.c instr.c win.c sim.c main.c
cflags = -lncurses

main: $(srcs)
	gcc $(cflags) $^ -o $@

.PHONY: clean
clean:
	rm main
