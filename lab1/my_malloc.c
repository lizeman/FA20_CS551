// THIS CODE IS MY OWN WORK, IT WAS WRITTEN WITHOUT CONSULTING
//
// A TUTOR OR CODE WRITTEN BY OTHER STUDENTS - Jiaying Lu
//

#include "my_malloc.h"
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>

#define SBRKDEFAULTSIZE 8192
#define CHUNKUNIT 8
#define SIGNATURE 0xffffffff

FreeListNode FREE_LIST_HEAD = NULL;
MyErrorNo my_errno=MYNOERROR;

// Help Functions
FreeListNode search_and_pop_free_list(size_t size) {
    FreeListNode valid = NULL;
    if (FREE_LIST_HEAD == NULL) { 
        return NULL;
    } else if (FREE_LIST_HEAD->size >= CHUNKHEADERSIZE + size) {
        valid = FREE_LIST_HEAD;
        FREE_LIST_HEAD = FREE_LIST_HEAD->flink;
        return valid;
    }

    FreeListNode prev = FREE_LIST_HEAD;
    while (prev->flink != NULL) {
        if (prev->flink->size >= CHUNKHEADERSIZE + size) {
            break;
        } else {
            prev = prev->flink;
        }
    }
    if (prev->flink != NULL) {
        // pop node
        valid = prev->flink;
        prev->flink = valid->flink;
        return valid;
    } else {
        return NULL;
    }
}

void add_node_to_free_list(void * chunk_head, size_t size) {
    FreeListNode node = (FreeListNode) chunk_head;
    node->size = size;

    if (FREE_LIST_HEAD == NULL) {
        FREE_LIST_HEAD = node;
        node->flink = NULL;
        return;
    } else if (FREE_LIST_HEAD > node) {
        node->flink = FREE_LIST_HEAD;
        FREE_LIST_HEAD = node;
        return;
    }
    
    FreeListNode prev = FREE_LIST_HEAD;
    while (prev->flink != NULL) {
        if (prev->flink > node) {
            node->flink = prev->flink;
            prev->flink = node;
            return;
        } else {
            prev = prev->flink;
        }
    }
    prev->flink = node;
    node->flink = NULL;
    return;
}

size_t split_chunk(void * chunk_head, size_t chunk_size, size_t request_size) {
    size_t remainder_size = chunk_size - request_size - CHUNKHEADERSIZE;
    if (remainder_size >= CHUNKHEADERSIZE + CHUNKUNIT) {
        // need split
        void* second_chunk_head = chunk_head + CHUNKHEADERSIZE + request_size;
        add_node_to_free_list(second_chunk_head, remainder_size);
        return request_size + CHUNKHEADERSIZE;
    } else {
        // no need to split
        return chunk_size;
    }
}

void sign_chunk_header(void * chunk_head, size_t actual_size) {
    uint32_t * p_head = (uint32_t *) chunk_head;
    *p_head++ = actual_size;
    *p_head = SIGNATURE;
    printf("[DEBUG] sign_chunk_header size=%u, signature=%u\n", *(uint32_t *)chunk_head, *p_head);
}

size_t get_free_list_len() {
    size_t len = 0;
    FreeListNode cur = FREE_LIST_HEAD;
    while (cur != NULL) {
        len++;
        cur = cur->flink;
    }
    return len;
}

size_t get_chunk_size(void * chunk_head) {
    uint32_t * p_head = (uint32_t *) chunk_head;
    size_t size = *p_head;
    printf("[DEBUG-get_chunk_size] *p_head=%u, size=%lu\n", *p_head, size);
    return size;
}
// End Help Functions

//my_malloc: returns a pointer to a chunk of heap allocated memory
void *my_malloc(size_t size) {
    printf("\nEnter my_malloc, request %lu memory\n", size);
    FreeListNode usable_node = NULL; 
    void * chunk_head = NULL;
    size_t actual_size = 0;
    if (size % CHUNKUNIT) size += (CHUNKUNIT - size % CHUNKUNIT);
    if ((usable_node = search_and_pop_free_list(size)) == NULL) {
        printf("can not find useable_node, address=%p\n", usable_node);
        if (size <= SBRKDEFAULTSIZE - CHUNKHEADERSIZE) {
            chunk_head = sbrk(SBRKDEFAULTSIZE);
            actual_size = split_chunk(chunk_head, SBRKDEFAULTSIZE, size);
            sign_chunk_header(chunk_head, actual_size);
            get_chunk_size(chunk_head);  //DEBUG
        } else {
            chunk_head = sbrk(size + CHUNKHEADERSIZE);
        }
    } else {
        printf("find useable_node, address=%p, size=%lu\n", usable_node, usable_node->size);
        chunk_head = (void *) usable_node;
        actual_size = split_chunk(chunk_head, usable_node->size, size);
        sign_chunk_header(chunk_head, actual_size);
        get_chunk_size(chunk_head);   //DEBUG
    }
    // debug
    FreeListNode head = free_list_begin();
    printf("[DEBUG] chunk_head=%p\n", chunk_head);
    printf("[DEBUG] free_list_begin=%p, len=%lu\n", head, get_free_list_len());
    printf("[DEBUG] free_list head size=%lu\n", head->size);
    // end debug
    return chunk_head +CHUNKHEADERSIZE;
}


//free_list_begin(): returns pointer to first chunk in free list
FreeListNode free_list_begin(void) {
    return FREE_LIST_HEAD;
}
