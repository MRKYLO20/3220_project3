
all:
	clang -Wall -g -shared -fPIC allocator.c -o libmyalloc.so -ldl

run:
	clang -Wall -g -o simple_test simple_test.c
	set environment LD_PRELOAD=./libmyalloc.so ./simple_test
	./simple_test

tar:
	tar cvzf project3.tgz README Makefile allocator.c

untar:
	tar xvzf project3.tgz