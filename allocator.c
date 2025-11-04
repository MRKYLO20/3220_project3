
#include <fcntl.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <string.h>

#define PAGENUMMAX = 10

typedef struct pageSizer {
    int pageSize;
    int * ptr;
} pageSizer;

pageSizer pageSizeTable[PAGENUMMAX];

for (int i; i < PAGENUMMAX; i++) {
    pageSizeTable[i].pageSize = 2**(i + 1);
    pageSizeTable[i].ptr = NULL;
}

/*
#define PAGESIZE 4096
// used for intitializing the memory to zero. (optional)
int fd = open("/dev/zero", O_RDWR);

// ask the OS to map a page of virtual memory
// initialized to zero (optional)
// initializing your memory may make debugging easier
void * page = mmap (NULL, PAGESIZE,
                    PROT_READ | PROT_WRITE,
                    MAP_PRIVATE, fd, 0);

// or if you don't want to zero out the memory
// you can do it like this

void * page = mmap (NULL, PAGESIZE,
                    PROT_READ | PROT_WRITE,
                    MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

// And, you can unmap the page, when you're done
munmap(page, PAGESIZE);
*/

void free(void *freePtr) {
    void (*original_free)(void *ptr);
    original_free = dlsym(RTLD_NEXT, "free");
    original_free(freePtr);
}

void *malloc(size_t size) {
    void *(*original_malloc)(size_t size);
    original_malloc = dlsym(RTLD_NEXT, "malloc");
    void *ptr = original_malloc(size);
    return ptr;
}

void *calloc(size_t count, size_t size) {
    size_t newSize = count * size;
    void *(*original_calloc)(size_t count, size_t size);
    original_calloc = dlsym(RTLD_NEXT, "calloc");
    void *ptr = original_calloc(count, size);
    return ptr;
}

void *realloc(void *ptr, size_t size) {
    /* Use the real realloc so contents are preserved, then update tracking */
    void *(*original_realloc)(void *ptr, size_t size);
    original_realloc = dlsym(RTLD_NEXT, "realloc");
    void *new_ptr = original_realloc(ptr, size);
    return new_ptr;
}