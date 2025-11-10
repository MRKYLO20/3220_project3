
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>

#define PAGENUMMAX 10
#define PAGESIZE 4096
#define nextPage 2

typedef struct sizeNode {
    int size;
    int * ptr;
} sizeNode;

typedef struct listNode {
    void * memoryBlock;
    struct listNode * nextNode;
} listNode;

sizeNode sizeTable[PAGENUMMAX];

int changed = 0;

void __attribute__((constructor)) library_init() {

    for (int i = 0; i < PAGENUMMAX; i++) {
        sizeTable[i].size = pow(2, i + 1);
        sizeTable[i].ptr = NULL;
    }

    //const char msg[] = "[myalloc] loaded\n";
    //(void)!write(STDERR_FILENO, msg, sizeof msg - 1);
}

void __attribute__((destructor)) library_cleanup() {
    //const char msg[] = "done\n";
    //(void)!write(STDERR_FILENO, msg, sizeof msg - 1);
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

void * getMemoryInPage(int index, int size) {
    void * page = sizeTable[index].ptr;
    if (page == NULL) {
        page = mmap (NULL, PAGESIZE,
            PROT_READ | PROT_WRITE,
            MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        
        //align
        int ptrSize = 8;

        int pos = ptrSize;
        int * blockSize = page;

        void * np = (char*)page + pos;
        pos = pos + ptrSize;

        listNode * free_list = (char*)page + pos;
        pos = pos + ptrSize;
        
        //initialize
        *blockSize = size;
        *np = NULL;

        listNode * firstNode = (char*)page + pos;
        pos = pos + sizeof(listNode);

        *free_list = firstNode;
        listNode * tempNode = firstNode;

        tempNode->memoryBlock = (char*)page + pos;
        pos = pos + size;
        
        //checking the space that the size would fill
        while (pos + size + (sizeof(listNode)) < PAGESIZE) {
            listNode * newNode = (char*)page + pos;
            pos = pos + (sizeof(listNode));

            tempNode->nextNode = newNode;
            tempNode = tempNode->nextNode;

            tempNode->memoryBlock = (char*)page + pos;
            pos = pos + size;
        }
    }
    page = mmap (NULL, PAGESIZE,
            PROT_READ | PROT_WRITE,
            MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    return page;
}


void * getMemory(size_t size) {
    
    /*int found = -1;
    int iterator = 0;
    while (found == -1 && iterator < PAGENUMMAX) {
        //const char msg2[] = "mallocing2\n";
        //(void)!write(STDERR_FILENO, msg2, sizeof msg2 - 1);
        if (size < sizeTable[iterator].size) {
            found = iterator;
        }
        else {
            iterator++;
        }
    }*/
    int index = ((__builtin_clz(size - 1) - 31) * -1);
    
    //if input is 0 function will give 31. Still going to give 2 bytes if requesting 0
    if (index == 31) {
        index = 0;
    }
    if (index >= 10) {
        int numPages = size / PAGESIZE;
        void * pages = mmap (NULL, numPages*PAGESIZE,
                    PROT_READ | PROT_WRITE,
                    MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        //const char msg2[] = "big mem\n";
        //(void)!write(STDERR_FILENO, msg2, sizeof msg2 - 1);
        return pages;
    }
    else {
        //const char msg2[] = "small mem\n";
        //(void)!write(STDERR_FILENO, msg2, sizeof msg2 - 1);
        return getMemoryInPage(index, size);
    }
}

void free(void *freePtr) {
    munmap(freePtr, PAGESIZE);
}

void *malloc(size_t size) {
    changed = 1;
    //const char msg[] = "mallocing\n";
    //(void)!write(STDERR_FILENO, msg, sizeof msg - 1);
    void * page = getMemory(size);
    //const char msg2[] = "mallocing2\n";
    //(void)!write(STDERR_FILENO, msg2, sizeof msg2 - 1);
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