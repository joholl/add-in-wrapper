#
#  Makefile of g1a-wrapper tool.
#

.PHONY: all

CC      = gcc
CCFLAGS = -Iinclude -W -Wall
CSRC    = $(shell find src/ -name '*.c')
COBJ    = $(patsubst src/%.c, build/%.o, $(CSRC))
CHDR    = $(shell find include/ -name '*.h')

OUTPUT  = build/g1a-wrapper

all: $(CHDR) $(OUTPUT)

install: all
	mkdir -p ~/bin
	cp $(OUTPUT) ~/bin

$(OUTPUT): $(COBJ)
	$(CC) $^ -o $@ $(CCFLAGS)

build/%.o: src/%.c
	$(CC) -c $< -o $@ $(CCFLAGS)

clear:
	rm -f bin/*.o

mrproper: clear
	rm -f $(OUTPUT)
