TARGET_PUBLISH = um
TARGET_DEBUG = debug_um
SRC = $(shell find . -name "*.c")
DEP = $(shell find . -name "*.h")
PREFIX ?= /usr/local
CFLAGS_PUBLISH	+= -march=native -Ofast -std=c11 -lm
CFLAGS_DEBUG	+= -Wall -W -pedantic -march=native -Ofast -std=c11 -lm

.PHONY: all install uninstall clean

all: $(TARGET_PUBLISH)

$(TARGET_PUBLISH): $(SRC) $(DEP)
	$(CC) $(CFLAGS_PUBLISH) $(SRC) -o $@

run: $(TARGET_PUBLISH)
	./$(TARGET_PUBLISH)

install: $(TARGET_PUBLISH)
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	install -m755 $(TARGET_PUBLISH) $(DESTDIR)$(PREFIX)/bin/$(TARGET_PUBLISH)

clean:
	$(RM) $(TARGET_PUBLISH)

uninstall:
	$(RM) $(DESTDIR)$(PREFIX)/bin/$(TARGET_PUBLISH)

$(TARGET_DEBUG): $(SRC) $(DEP)
	$(CC) $(CFLAGS_DEBUG) $(SRC) -o $@

debug: $(TARGET_DEBUG)

rundebug: $(TARGET_DEBUG)
	./$(TARGET_DEBUG)
