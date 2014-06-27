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

int testMemoryWorst() {
	//free(malloc(1000000000));
	int i;
	int memoryNeeded = 13371337;
	for (i = 0; i < memoryNeeded; i++) {
		malloc(1);
	}	
	return memoryNeeded;
}

int main() {
	test(testMemoryWorst);
}
