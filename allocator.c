
#include <fcntl.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <string.h>

#define PAGENUMMAX 10

typedef struct pageSizer {
    int pageSize;
    int * ptr;
} pageSizer;

pageSizer pageSizeTable[PAGENUMMAX];

/*for (int i = 0; i < PAGENUMMAX; i++) {
    pageSizeTable[i].pageSize = 2**(i + 1);
    pageSizeTable[i].ptr = NULL;
}*/

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

}

void *malloc(size_t size) {
    void *ptr = NULL;
    return ptr;
}

void *calloc(size_t count, size_t size) {
    void *ptr = NULL;
    return ptr;
}

void *realloc(void *ptr, size_t size) {
    void *new_ptr = NULL;
    return new_ptr;
}