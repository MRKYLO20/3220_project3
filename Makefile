all:
	clang -Wall -g -shared -fPIC allocator.c -o libmyalloc.so -ldl -lm

set:
	clang -Wall -g -shared -fPIC allocator.c -o libmyalloc.so -ldl -lm
	clang -Wall -g -O0 -o simple_test simple_test.c

run:
	LD_PRELOAD=./libmyalloc.so ./simple_test

gdb:
	gdb ./simple_test -x init.gdb

tar:
	tar cvzf project3.tgz README Makefile allocator.c

untar:
	tar xvzf project3.tgz