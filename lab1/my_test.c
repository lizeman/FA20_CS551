#include <stdio.h>
#include "my_malloc.h"
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>

size_t get_free_list_len() {
    size_t len = 0;
    FreeListNode cur = free_list_begin();
    while (cur != NULL) {
        len++;
        cur = cur->flink;
    }
    return len;
}

void print_debug_info() {
    printf("my_errno=%d\n", my_errno);
    printf("free_list len=%lu: \n", get_free_list_len());
    // traverse
    FreeListNode head = free_list_begin();
    int i = 0;
    while (head != NULL) {
        printf("|--#%d node size=%lu,addr=%p\n", i++, head->size, head);
        head = head->flink;
    }
}

int main (char argc, char *argv[]) {
    /* Case #1: 
     * malloc one chunk,
     * then free it, and coalesce_free_list.
     */
    void * ptr = my_malloc(8);
    printf("===after ptr=malloc(8) ===\n");
    print_debug_info();
    my_free(ptr);
    printf("===after free(ptr) ===\n");
    print_debug_info();
    coalesce_free_list();
    printf("===after coalesce_free_list(), free_list_len should be 1; ===\n");
    print_debug_info();

    /* Case #2: 
     * free an unvalid point
     */
    printf("\n=== Try to free unvalid point===\n");
    printf("my_free( free_list_begin()), addr=%p\n", free_list_begin());
    my_free(free_list_begin());
    print_debug_info();
    printf("my_free( free_list_begin()+16), addr+16=%p\n", free_list_begin()+16);
    my_free(free_list_begin() + 16);
    print_debug_info();
    printf("my_free() an officially malloced pointer\n");
    int * iptr = malloc(4 * sizeof(int));
    iptr[0] = 1;
    iptr[1] = 2;
    iptr[2] = 3;
    iptr[3] = 4;
    my_free(iptr);
    print_debug_info();
    free(iptr);
    printf("my_free() pointer beyond heap high end\n");
    my_free(sbrk(0) + 10);
    print_debug_info();

    /* Case #3:
     * malloc zero size chunk
     */
    ptr = my_malloc(0);
    printf("\n===after ptr=malloc(0) ===\n");
    print_debug_info();
    my_free(ptr);
    printf("===after free(ptr), should have two free_list_node ===\n");
    print_debug_info();

    /* Case #4:
     * malloc a small chunk from existing free_list_node
     */
    ptr = my_malloc(1024);
    printf("\n===after ptr=malloc(1024), second node should be splited ===\n");
    print_debug_info();
    printf("===after free(ptr), should have three free_list_node ===\n");
    my_free(ptr);
    print_debug_info();

    /* Case #5:
     * malloc a oversized chunk from existing free_list_node,
     * such node would not be splited
     */
    ptr = my_malloc(7128);
    printf("\n===after ptr=malloc(7128), third node be occupied, and NOT be splited ===\n");
    print_debug_info();
    printf("===after free(ptr), should have three free_list_node, and third node size=7144>7128+8 ===\n");
    my_free(ptr);
    print_debug_info();


    /* Case #6:
     * malloc a chunk with size >= 8192
     * no free_list_node would be added
     */
    ptr = my_malloc(8192);
    printf("\n===after ptr=malloc(8192), directly sbrk(8192+8), no node added ===\n");
    print_debug_info();
    printf("===after free(ptr), should have four free_list_node, and fourth node size=8192+8 ===\n");
    my_free(ptr);
    print_debug_info();

    /* Case #7
     * Request a non 8 aligned size
     */
    void * ptr1 = my_malloc(15);
    printf("\n===after ptr1=malloc(15) ===\n");
    print_debug_info();
    printf("===after free(ptr1), should have five free_list_node, and second node size=16+8 ===\n");
    my_free(ptr1);
    print_debug_info();

    /* Case #8
     * coalesce_free_list for all nodes
     */
    printf("\n===after coalesce_free_list(), free_list_len should be 2; ===\n");
    printf("the node size should be 8192, 8200\n");
    coalesce_free_list();
    print_debug_info();

    /* Case #9
     * malloc int*
     */
    printf("\n===malloc int array===\n");
    iptr = my_malloc(4 * sizeof(int));
    iptr[0] = 1;
    iptr[1] = 2;
    iptr[2] = 3;
    iptr[3] = 4;
    assert(iptr[0] == 1);
    assert(iptr[1] == 2);
    assert(iptr[2] == 3);
    assert(iptr[3] == 4);
    my_free(iptr);
    print_debug_info();
}
