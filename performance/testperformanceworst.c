#include <malloc.h>

int main() {
    int size = 1000;
    int i;
    char ** list = (char **)malloc(size*sizeof(char *));
    //void *p = malloc(1024 * 20); /* This line is used to destroy performance is memory goes in weird direction */
    
    for (i = 0; i<size; i++) list[i] = (char *)malloc(1); /* allokera massa små block */
    for (i = 1; i < size; i += 2) free(list[i]); /* Fria vart annat */
    
    //free(p);
    for (i=0;i<1000000;i++) free(malloc(10));/*Testet som faktiskt tar tid */
}
