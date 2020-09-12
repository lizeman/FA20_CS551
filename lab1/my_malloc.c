// THIS CODE IS MY OWN WORK, IT WAS WRITTEN WITHOUT CONSULTING
//
// A TUTOR OR CODE WRITTEN BY OTHER STUDENTS - Jiaying Lu
//

#include <stdio.h>
#include "my_malloc.h"
#include <unistd.h>
#include <stdint.h>

#define SBRKDEFAULTSIZE 8192
#define CHUNKUNIT 8
#define MINCHUNKSIZE 16
#define SIGNATURE 0xabcdef27

#define max(a,b)             \
({                           \
    __typeof__ (a) _a = (a); \
    __typeof__ (b) _b = (b); \
    _a > _b ? _a : _b;       \
})

#define min(a,b)             \
({                           \
    __typeof__ (a) _a = (a); \
    __typeof__ (b) _b = (b); \
    _a < _b ? _a : _b;       \
})

FreeListNode FREE_LIST_HEAD = NULL;
MyErrorNo my_errno=MYNOERROR;
void * heap_low_end = NULL;    // for my_free() safety

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

size_t get_rounded_freelistnode_size() {
    size_t freelistnode_round = sizeof(struct freelistnode);
    if (freelistnode_round % CHUNKUNIT) 
        freelistnode_round += (CHUNKUNIT - freelistnode_round % CHUNKUNIT);
    return freelistnode_round;
}

size_t split_chunk(void * chunk_head, size_t chunk_size, size_t request_size) {
    size_t remainder_size = chunk_size - request_size - CHUNKHEADERSIZE;
    if (remainder_size >= max(MINCHUNKSIZE, get_rounded_freelistnode_size())) {
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
}

// End Help Functions

//my_malloc: returns a pointer to a chunk of heap allocated memory
void *my_malloc(size_t size) {
    if (heap_low_end == NULL) heap_low_end = sbrk(0);
    FreeListNode usable_node = NULL; 
    void * chunk_head = NULL;
    size_t actual_size = 0;
    if (size % CHUNKUNIT) size += (CHUNKUNIT - size % CHUNKUNIT);
    size = max(size, max(MINCHUNKSIZE, get_rounded_freelistnode_size())-CHUNKHEADERSIZE);
    if ((usable_node = search_and_pop_free_list(size)) == NULL) {
        if (size <= SBRKDEFAULTSIZE - CHUNKHEADERSIZE) {
            if ((chunk_head = sbrk(SBRKDEFAULTSIZE)) == (void *)-1) {
                my_errno = MYENOMEM;
                return NULL;
            }
            actual_size = split_chunk(chunk_head, SBRKDEFAULTSIZE, size);
        } else {
            if ((chunk_head = sbrk(size + CHUNKHEADERSIZE)) == (void *)-1) {
                my_errno = MYENOMEM;
                return NULL;
            }
            actual_size = size + CHUNKHEADERSIZE;
        }
    } else {
        chunk_head = (void *) usable_node;
        actual_size = split_chunk(chunk_head, usable_node->size, size);
    }
    sign_chunk_header(chunk_head, actual_size);
    my_errno = MYNOERROR;
    return chunk_head + CHUNKHEADERSIZE;
}

//my_free: reclaims the previously allocated chunk referenced by ptr
void my_free(void *ptr) {
    if (heap_low_end == NULL || ptr - CHUNKHEADERSIZE < heap_low_end) {
        my_errno = MYBADFREEPTR;
        return;
    }
    ptr -= CHUNKHEADERSIZE;
    uint32_t * p_head = (uint32_t *) ptr;
    if (*++p_head != SIGNATURE) {
        my_errno = MYBADFREEPTR;
        return;
    }
    add_node_to_free_list(ptr, *--p_head);
    my_errno = MYNOERROR;
}


//free_list_begin(): returns pointer to first chunk in free list
FreeListNode free_list_begin(void) {
    return FREE_LIST_HEAD;
}

//coalesce_free_list(): merge adjacent chunks on the free list
void coalesce_free_list(void) {
    FreeListNode node = FREE_LIST_HEAD;
    while (node != NULL) {
        if ((void *)node + node->size == node->flink) {
            node->size += node->flink->size;
            node->flink = node->flink->flink;
        } else {
            node = node->flink;
        }
    }
}
