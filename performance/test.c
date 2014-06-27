#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <unistd.h>
#ifdef EGEN
#include "../malloc.h"
#else
#include <malloc.h>
#endif
#include <string.h>

int memoryTotal() {
	struct mallinfo mi;
	mi = mallinfo();
	return mi.arena + mi.hblkhd;
}

void mallocVersion() {
	printf("Whos: %d\n", mallinfo().keepcost);
}

int startMemory = -1;

int memoryAllocated() {
	int total = memoryTotal();
	if (startMemory == -1) startMemory = total;
	return total - startMemory;
}


int test(int(*func)(void)) {
	mallocVersion();
	memoryAllocated();
	
	int memoryNeeded = func();
	int allocated = memoryAllocated();

	printf("Free blocks: %d\n", mallinfo().ordblks + mallinfo().smblks);
	printf("Memory allocated: %d\n", allocated);
	printf("Memory needed: %d\n", memoryNeeded);
	printf("Memory percentage: %.2fx more than needed\n", allocated / (double)memoryNeeded);
}
