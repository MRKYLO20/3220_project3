
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
#define BIGBLOCKMAX 30

typedef struct sizeNode {
    int size;
    int * ptr;
} sizeNode;

typedef struct listNode {
    struct listNode * nextNode;
    void * memoryBlock;
} listNode;

typedef struct headerStruct {
    int blockSize;
    void * np;
    void * freeList;
    void * usedList;
} headerStruct;

//table to get which size chunks to allocate
static sizeNode sizeTable[PAGENUMMAX];

//list of big blocks that the allocator may need to select
static void * bigBlocks[BIGBLOCKMAX];
static int bigBlockPos = 0;

void __attribute__((constructor)) library_init() {
    for (int i = 0; i < PAGENUMMAX; i++) {
        sizeTable[i].size = pow(2, i + 1);
        sizeTable[i].ptr = NULL;
    }
    for (int i = 0; i < BIGBLOCKMAX; i++) {
        bigBlocks[i] = NULL;
    }
}

void * freeChunk(void * chunk) {
    int found = 0;
    while (found == 0) {

    }
    return NULL;
}

void * getChunk(void * page) {

    void * freeList = ((headerStruct *)page)->freeList;
    void * usedList = ((headerStruct *)page)->usedList;

    listNode * target = freeList;
    freeList = ((listNode*)freeList)->nextNode;

    if (usedList == NULL) {
        usedList = target;
    }
    else {
        listNode * temp = usedList;
        usedList = target;
        target->nextNode = temp;
    }
    return target->memoryBlock;
}

void * createNewPage(int chunkSize) {
    void * page = mmap (NULL, PAGESIZE,
        PROT_READ | PROT_WRITE,
        MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    
    //align header
    int pos = 0;
    headerStruct * header = page;
    pos = pos + sizeof(headerStruct);
    
    //initialize
    header->blockSize = chunkSize;


    //set first node in sequence
    listNode * firstNode = (listNode *)((char*)page + pos);
    pos = pos + sizeof(listNode);

    //set freeList initially to the first node
    header->freeList = (void *)(firstNode);
    listNode * tempNode = firstNode;
    
    //align memory block
    tempNode->memoryBlock = (char*)page + pos;
    pos = pos + chunkSize;

    //list runs like nextNode(8)|memoryBlockptr(8)|memory

    //checking the space that the size would fill
    while (pos + chunkSize + (sizeof(listNode)) < PAGESIZE) {
        listNode * newNode = (listNode *)((char*)page + pos);
        pos = pos + (sizeof(listNode));

        tempNode->nextNode = newNode;
        tempNode = tempNode->nextNode;

        tempNode->memoryBlock = (char*)page + pos;
        pos = pos + chunkSize;

    }
    tempNode->nextNode = NULL;

    return page;
}

void * getMemoryInPage(int index, int size) {
    void * page = sizeTable[index].ptr;
    int chunkSize = sizeTable[index].size;
    int success = 0;
    while (success == 0) {
        //if there is no page create a new one
        if (page == NULL) {
            page = createNewPage(chunkSize);
            sizeTable[index].ptr = page;
        }
        //get header vars
        
        void * freeList = ((headerStruct *)page)->freeList;
        //verified the same location as in created page
        //if there is nothing free, move to a new page
        if (freeList != NULL) {
            void * targetBlock = getChunk(page);
            success = 1;
            return targetBlock;
        }
        else {
            page = createNewPage(chunkSize);
            ((headerStruct *)page)->np = page;
        }
    }

    return page;
}

void * getMemory(size_t size) {
    
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
        bigBlocks[bigBlockPos] = pages;
        bigBlockPos++;
        return pages;
    }
    else {
        return getMemoryInPage(index, size);
    }
}

void free(void *freePtr) {
    void * block = bigBlocks[0];
    int found = 0;
    while (block != NULL) {
        if (block == freePtr) {
            munmap(freePtr, PAGESIZE);
            found = 1;
            break;
        }
        else {
            block = block + 1;
        }
    }
    if (found == 0) {
        munmap(freePtr, PAGESIZE);
    }
}

void *malloc(size_t size) {
    void * block = getMemory(size);
    return block;
}

void *calloc(size_t count, size_t size) {
    void * block = getMemory(size * count);
    return block;
}

void *realloc(void *ptr, size_t size) {
    free(ptr);
    void * block = malloc(size);
    return block;
}