
#include <fcntl.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#define PAGENUMMAX 10
#define PAGESIZE 4096

typedef struct sizeNode {
    int size;
    int * ptr;
} sizeNode;

sizeNode sizeTable[PAGENUMMAX];

void __attribute__((constructor)) library_init() {
    printf("Loading library.\n");
    for (int i = 0; i < PAGENUMMAX; i++) {
        sizeTable[i].size = pow(i + 1, 2);
        sizeTable[i].ptr = NULL;
    }
}

void __attribute__((destructor)) library_cleanup() {
    printf("Unloading library.\n");
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

/*void * getMemoryInPage(index, size) {
    void * cheese = NULL;
    return cheese;
}*/


void * getMemory(size_t size) {
    
    int found = -1;
    int iterator = 0;
    while (found == -1 || iterator < PAGENUMMAX) {
        if (size < sizeTable[iterator].size) {
            found = iterator;
        }
        else {
            iterator++;
        }
    }
    if (found == -1) {
        int numPages = size / PAGESIZE;
        void * pages = mmap (NULL, numPages*PAGESIZE,
                    PROT_READ | PROT_WRITE,
                    MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        return pages;
    }
    else {
        void * cheese = NULL;
        return cheese;
        //return getMemoryInPage(found, size);
    }
}

void free(void *freePtr) {
    munmap(freePtr, PAGESIZE);
}

void *malloc(size_t size) {
    void * page = getMemory(size);
    return page;
}

void *calloc(size_t count, size_t size) {
    void * page = getMemory(size);
    return page;
}

void *realloc(void *ptr, size_t size) {
    free(ptr);
    void * new_ptr = malloc(size);
    return new_ptr;
}