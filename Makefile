TARGET = um
SRC = $(shell find . -name "*.c")
DEP = $(shell find . -name "*.h")
PREFIX ?= /usr/local
CFLAGS += -Wall -W -pedantic -march=native -Ofast -std=c11 -lm

.PHONY: all install uninstall clean

all: $(TARGET)

$(TARGET): $(SRC) $(DEP)
	$(CC) $(CFLAGS) $(SRC) -o $@

run: um
	./um

install: $(TARGET)
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	install -m755 $(TARGET) $(DESTDIR)$(PREFIX)/bin/$(TARGET)

clean:
	$(RM) $(TARGET)

uninstall:
	$(RM) $(DESTDIR)$(PREFIX)/bin/$(TARGET)
