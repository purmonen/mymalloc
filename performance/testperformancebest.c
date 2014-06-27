#include <malloc.h>

int main() {
    int i;
    for (i = 0; i < 1000; i ++) malloc(1024*1024);
}
