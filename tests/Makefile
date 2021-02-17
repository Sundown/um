CFLAGS += -Wall -W -pedantic -march=native -Ofast -std=c11 -lm

.PHONY: math clean

math: math.c
	$(CC) $(CFLAGS) $^ -o $@
clean:
	$(RM) math
