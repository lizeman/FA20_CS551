#include "my_malloc.h"
#include <stdio.h>

int main (char argc, char *argv[]) {
    void * ptr = my_malloc(8);
    printf("[DEBUG] my_malloc return ptr=%p\n", ptr);

    void * ptr_1 = my_malloc(800);
    printf("[DEBUG] my_malloc return ptr=%p\n", ptr_1);
}
