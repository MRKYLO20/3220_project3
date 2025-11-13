
set pagination off
set breakpoint pending on
set debuginfod enabled on
set environment LD_PRELOAD=./libmyalloc.so
set startup-with-shell off
break main
break 85
break allocator.c:216

break allocator.c:169
break allocator.c:192
break allocator.c:59
