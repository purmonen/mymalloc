#include "malloc.h"
#include <sys/mman.h>
#include <stdio.h>
#include <errno.h>
#include "brk.h"
#include <unistd.h>
#include <math.h>
#include <assert.h>

#define ALIGNMENT 8
#define ALIGNMENTMASK ~7
#define HEADERSIZE ((sizeof(Header)+ALIGNMENT-1) & ALIGNMENTMASK)

/*
 * Header is a structure for storing a linked list of nodes
 * that are associated with memory that can be allocated by
 * users of malloc.
 */

typedef struct Header Header;
struct Header
{
    struct Header *next;  /* Next element in linked list */
    size_t size;          /* Size that can this node has that can be allocated by user */
};


/*
 * Head of the free list
 */
Header *freeList = NULL;

/*
 * Stores how much memory has been allocated by mmap
 */
static void * __endHeap;

/** endHeap
 *
 * Returns __endHeap or initialize if its not
 */
void *endHeap()
{
    if(__endHeap == 0) __endHeap = sbrk(0);
    return __endHeap;
}

/** clonestAlignedAddress
 *
 * takes and address and returns the closest larger
 * address aligned with ALIGNMENT
 */
long
closestAlignedAddress(
                           long address) /* Address to be aligned */
{
    long nextAddress = address - (address % ALIGNMENT);
    if (nextAddress < address) {
        nextAddress += ALIGNMENT;
    }
    return nextAddress;
}


/** getNextHeader
 *
 * Returns the closest header that can fit within address start and end
 * and be aligned and still have empty space available
 */
Header *
getNextHeader(
              long start, /* start address */
              long end)   /* end address */
{
    long nextPosition = closestAlignedAddress(start);
    long emptySpace = end - nextPosition - HEADERSIZE;
    
    if (emptySpace <= 0) return NULL;
    Header *header = (Header *)nextPosition;
    header->size = emptySpace;
    header->next = NULL;
    return header;
}

/** insertIntoList
 *
 * insert a new header into the list and merges the new element
 * if possible
 */
void insertIntoList(
                    Header **list,     /* List to be inserted into */
                    Header *header)    /* Header to be inserted */
{
    header->next = NULL;
    Header *current = *list;
    Header *prev = NULL;
    while (current && current < header) {
        prev = current;
        current = current->next;
    }
    header->next = current;
    
    /* Try to merge header with the next element if possible */
    if (header->next) {
        if ((long)header + header->size + HEADERSIZE == (long)header->next) {
            header->size += HEADERSIZE + header->next->size;
            header->next = header->next->next;
        }
    }
    
    /* If previous element exist try to merge that one with the new element */
    if (prev) {
        prev->next = header;
        if ((long)prev + prev->size + HEADERSIZE == (long)header) {
            prev->size += HEADERSIZE + header->size;
            prev->next = header->next;
        }
    } else {
        *list = header;
    }
}

/** removeFromList
 *
 * Removes header from list
 */
void removeFromList(
                    Header **list,    /* List to remove header from */
					Header *beforeHeader,
                    Header *header) { /* Header to be removed */
    if (!header) return;
    if (beforeHeader) {
        beforeHeader->next = header->next;
    } else {
        *list = header->next;
    }
}

/* addRemainingHeaderToList
 *
 * If a new header can be fitted into the headers place when header is 
 * resized to size then the header is resized to size and the new header
 * is inserted into the list
 */
void addRemainingHeaderToList(Header **list,  /* List to use */
                              Header *header, /* Header to resize */
                              size_t size)    /* size to resize */
{
    long start = (long)header + size + HEADERSIZE;
    long end = (long)header + header->size + HEADERSIZE;
    Header * nextHeader = getNextHeader(start, end);
    if (nextHeader) {
        header->size = size;
        insertIntoList(list, nextHeader);
    }
}

/** firstFitHeaderFromList
 *
 * returns the first header from list that has atleast size
 * available memory
 */
Header * firstFitHeaderFromList(
                                Header **list, /* List to use */
                                size_t size) { /* Size needed */
    Header *current = *list;
	Header *prev = NULL;
    while (current) {
        if (current->size >= size) {
            removeFromList(list, prev, current);
            
            /* Add header with remaining space if there is room for one */
            addRemainingHeaderToList(list, current, size);
            
            /* found matching header */
            return current;
        }
        prev = current;
        current = current->next;
    }
    return NULL;
}

/** bestFitHeaderFromList
 *
 * returns the header that is closes to size but least as large
 */
Header * bestFitHeaderFromList(
                               Header **list, /* List to use */
                               size_t size) { /* size needed */
    Header *current = *list;
	Header *prev = NULL;
    Header *bestHeader = NULL;
	Header *beforeBestHeader = NULL;
    while (current) {
        if (current->size >= size) {
            /* Change currently found best header if we find a better match or if there was no
             * previously used
             */
            if (!bestHeader || (bestHeader->size - size) > (current->size - size)) {
				beforeBestHeader = prev;
                bestHeader = current;
            }
			if (bestHeader->size == size) {
				break;
			}
        }
		prev = current;
        current = current->next;
    }
    if (!bestHeader) return NULL;
    /* Remove best header from free list */
    removeFromList(list, beforeBestHeader, bestHeader);
    
    /* add new header with remaining space if there is enough */
    addRemainingHeaderToList(list, bestHeader, size);
    return bestHeader;
}

int firstRun = 0;

/** getEmptyHeaderFromList
 *
 * tries to find an header in a list that has appropriate size with either
 * firstFit or bestFit
 */
Header * getEmptyHeaderFromList(
                                Header **list, /* List to use */
                                size_t size)   /* Size needed */
{
#if STRATEGY == 2
	if (!firstRun) printf("BEST FIT\n");
	firstRun = 1;
    return bestFitHeaderFromList(list, size);
#else
	if (!firstRun) printf("FIRST FIT\n");
	firstRun = 1;
    return firstFitHeaderFromList(list, size);
#endif
}

/** allocateMoreSpace
 *
 * allocate enough space so that size request can be fulfilled and atleast one page size
 * and insert a new header with that size
 */
void allocateMoreSpace(
                       size_t size) /* Size needed */
{
    long pageSize = sysconf(_SC_PAGESIZE);
    long usedSize = (int)size + HEADERSIZE;
    long pages = (usedSize - 1) / pageSize + 1;
    long totalSize = pages * pageSize;
    Header * header = mmap(__endHeap, totalSize, PROT_READ|PROT_WRITE, MAP_ANON|MAP_SHARED, -1, 0);
    __endHeap += totalSize;
    if (!header || header == (Header *)-1) return;
    header->size = totalSize - HEADERSIZE;
    insertIntoList(&freeList, header);
}

/** headerFromAddress
 *
 * returns a header from an address where headers storage starts
 */
Header * headerFromAddress(
                           void *address) /* address to use */
{
    return (Header *)((char *)address - HEADERSIZE);
}

/** addressFromHeader
 *
 * returns the address from a header where storage starts
 */
void * addressFromHeader(Header *header) {
    return (void *)((char *)header + HEADERSIZE);
}

/** malloc
 *
 * return an address where the caller can store memory
 */
void * malloc(
              size_t size) /* Size of memory needed */
{
    if (size <= 0) return NULL;
    
    /* Try to find empty header in free list if not possible allocate new space */
    Header * emptyHeader = getEmptyHeaderFromList(&freeList, size);
    if (!emptyHeader) {
        allocateMoreSpace(size);
        emptyHeader = getEmptyHeaderFromList(&freeList, size);
        if (!emptyHeader) {
			return NULL;
		}
    }
    return addressFromHeader(emptyHeader);
}

/** realloc
 *
 * allocates new memory with malloc and copies content from pointer p to it
 */
void * realloc(void *p,        /* address to be copied from */
               size_t size) {  /* size to be copied */
    if (size <= 0) {
        free(p);
        return NULL;
    }
    if (!p) return malloc(size);
    Header *header = headerFromAddress(p);
    void *data = malloc(size);
    
    /* Only copy enough memory to not exceed boundaries */
    long minSize = size < header->size ? size : header->size;
    long i;
    for (i = 0; i < minSize; i++) {
        ((char *)data)[i] = ((char *)p)[i];
    }
    free(p);
    return data;
}

/** free
 *
 * frees content allocated with malloc so it can be reused
 */
void free(
          void *p) /* Address to be freed */
{
    if (!p) return;
    Header *header = headerFromAddress(p);
    insertIntoList(&freeList, header);
}

int countFreeList() {
	Header *current = freeList;
	int count = 0;
	while (current) {
		current = current->next;
		count++;	
	}
	return count;
}

struct mallinfo mallinfo() {
	struct mallinfo info;
	info.arena = 0;
	info.hblkhd = (int) (long)endHeap();
	info.ordblks = countFreeList();
	info.smblks = 0;
	info.keepcost = -1337;
	return info;
}
