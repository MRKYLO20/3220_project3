all:
	clang -Wall -g -shared -fPIC allocator.c -o libmyalloc.so -ldl -lm

set:
	clang -Wall -g -shared -fPIC allocator.c -o libmyalloc.so -ldl -lm
	clang -Wall -g -O0 -o simple_test simple_test.c

set2:
	clang -Wall -g -shared -fPIC allocator.c -o libmyalloc.so -ldl -lm
	clang -Wall -g -O0 -o simple_test2 simple_test2.c

run:
	LD_PRELOAD=./libmyalloc.so ./simple_test

run2:
	LD_PRELOAD=./libmyalloc.so ./simple_test2

valgrind:
	valgrind --leak-check=full --track-origins=yes env LD_PRELOAD=./libmyalloc.so ./simple_test

gdb:
	gdb ./simple_test -x init.gdb

gdb2:
	gdb ./simple_test2 -x init2.gdb

tar:
	tar cvzf project3.tgz README Makefile allocator.c

untar:
	tar xvzf project3.tgz