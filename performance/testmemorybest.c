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
#include "test.c"

int testMemoryBest() {
	int pageSize = sysconf(_SC_PAGESIZE);
	int headerSize = 16;
	int count = 10000;
	int size = pageSize - headerSize;
	int i;
	for (i = 0; i < count; i++) {
		malloc(size);
	}	
	return size * count;
}

int main() {
	test(testMemoryBest);
}
