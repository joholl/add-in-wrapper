#
#  Makefile of g1a-wrapper tool.
#

.PHONY: all install clean mrproper

cc    = gcc
as    = as
flags = -Iinclude -W -Wall
obj   = build/bmp_utils.o build/g1a-wrapper.o build/error.o
hdr   = include/bmp_utils.h include/g1a-wrapper.h include/error.h

output = build/g1a-wrapper

all: build $(hdr) $(output)

install:
	sudo cp $(output) ~/bin

build:
	mkdir -p build

$(output): $(obj)
	$(cc) $^ -o $@ $(flags)

build/%.o: src/%.c
	$(cc) -c $^ -o $@ $(flags)

clean:
	rm -f build/*.o

mrproper: clean
	rm -f $(output)
