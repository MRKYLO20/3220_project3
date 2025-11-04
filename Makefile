
all:
	gcc -Wall -g -shared -fPIC allocator.c -o libmyalloc.so -ldl
	set environment LD_PRELOAD=./libmyalloc.so

run:
	gcc -Wall -g -o simple_test simple_test.c
	./simple_test

tar:
	tar cvzf project3.tgz README Makefile allocator.c

untar:
	tar xvzf project3.tgz