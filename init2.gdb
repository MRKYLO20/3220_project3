
set pagination off
set breakpoint pending on
set debuginfod enabled on
set environment LD_PRELOAD=./libmyalloc.so
set startup-with-shell off
break main
break allocator.c:124
break allocator.c:185
break allocator.c:201