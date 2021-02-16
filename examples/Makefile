CFLAGS += -Wall -W -pedantic -march=native -Ofast -std=c11 -lm

.PHONY: all repl standalone clean

all: repl embed standalone

repl: repl.c
	$(CC) $(CFLAGS) $^ -o $@

embed: embed.c
	$(CC) $(CFLAGS) $^ -o $@

standalone: standalone.c
	$(CC) $(CFLAGS) $^ -o $@

clean:
	$(RM) repl embed standalone
