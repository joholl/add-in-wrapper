.PHONY: clean mrproper

CC     = gcc
CFLAGS = -Iinclude -Wall -Werror

all: bin/bmp_utils.o bin/c_g1awrapper.o
	$(CC) -o bin/c_g1awrapper $^

bin/c_g1awrapper.o : include/bmp_utils.h include/g1a_header.h
	$(CC) -c src/c_g1awrapper.c -o bin/c_g1awrapper.o $(CFLAGS)

bin/bmp_utils.o : include/bmp_utils.h
	$(CC) -c src/bmp_utils.c -o bin/bmp_utils.o $(CFLAGS)

clean:
	rm bin/*.o

mrproper: clean
	rm bin/c_g1awrapper

realclean: mrproper

install: all
	cp bin/c_g1awrapper /usr/bin
