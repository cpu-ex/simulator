srcs = mem.c core.c instr.c main.c

main: $(srcs)
	gcc $^ -o $@

.PHONY: clean
clean:
	rm main
