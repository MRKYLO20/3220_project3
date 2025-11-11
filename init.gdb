
set pagination off
set breakpoint pending on
set debuginfod enabled on
set environment LD_PRELOAD=./libmyalloc.so
set startup-with-shell off
break main
break simple_test.c:34