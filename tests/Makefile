CFLAGS += -Wall -W -pedantic -march=native -Ofast -std=c11 -lm

.PHONY: all repl standalone clean

a

math: math.c
	$(CC) $(CFLAGS) $^ -o $@
clean:
	$(RM) repl embed standalone
