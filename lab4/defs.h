// THIS CODE IS MY OWN WORK, IT WAS WRITTEN WITHOUT CONSULTING
// 
// A TUTOR OR CODE WRITTEN BY OTHER STUDENTS - Jiaying Lu
//
#ifndef defs_h
#define defs_h

#include <stdlib.h>
#include <sys/types.h>

#define KEY (key_t)78738
#define BMAP_SIZE (size_t)(1 << 22)
#define MAX_PRC_NUM 20
#define SHM_SIZE BMAP_SIZE + MAX_PRC_NUM * sizeof(int) + MAX_PRC_NUM * sizeof(process_t) + sizeof(pid_t) + sizeof(process_t)
#define MSG_TYPE_REGISTER 1
#define MSG_TYPE_PREPORT 1

typedef struct {
    long type;
    int data;
} msg_t;

typedef struct {
    pid_t pid;
    int tested;   // number of candidates tested
    int skipped;  // number of candidates skipped
    int found;    // number of perfect number found
} process_t;


#endif  /* defs_h */
