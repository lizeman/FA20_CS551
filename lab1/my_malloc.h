//
//  my_malloc.h
//  Lab1: Malloc
//
//  Created by Dorian Arnold on 7/17/20.
//  Copyright © 2020 Dorian Arnold. All rights reserved.
//

#ifndef my_malloc_h
#define my_malloc_h

//the size of the header for heap allocated memory chunks
#define CHUNKHEADERSIZE 8

//error signaling
typedef enum {MYNOERROR, MYENOMEM, MYBADFREEPTR } MyErrorNo;
//MyErrorNo my_errno=MYNOERROR;

//my_malloc: returns a pointer to a chunk of heap allocated memory
void *my_malloc(size_t size);

//my_free: reclaims the previously allocated chunk referenced by ptr
void my_free(void *ptr);

//struct freelistnode: node for linked list of 'free' chunks
typedef struct freelistnode {
    struct freelistnode *flink; //pointer to next free chunk node
    size_t size; //size of current chunk
} * FreeListNode;

//free_list_begin(): returns pointer to first chunk in free list
FreeListNode free_list_begin(void);

//coalesce_free_list(): merge adjacent chunks on the free list
void coalesce_free_list(void);

#endif /* my_malloc_h */
