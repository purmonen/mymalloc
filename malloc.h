#include <stddef.h>

void * malloc(size_t);
void * realloc(void *, size_t);
void free(void *);
void printList();
void * endHeap();

struct mallinfo {
   int arena;     /* Non-mmapped space allocated (bytes) */
   int ordblks;   /* Number of free chunks */
   int smblks;    /* Number of free fastbin blocks */
   int hblks;     /* Number of mmapped regions */
   int hblkhd;    /* Space allocated in mmapped regions (bytes) */
   int usmblks;   /* Maximum total allocated space (bytes) */
   int fsmblks;   /* Space in freed fastbin blocks (bytes) */
   int uordblks;  /* Total allocated space (bytes) */
   int fordblks;  /* Total free space (bytes) */
   int keepcost;  /* Top-most, releasable space (bytes) */
};	
struct mallinfo mallinfo();
