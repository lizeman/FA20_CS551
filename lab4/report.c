// THIS CODE IS MY OWN WORK, IT WAS WRITTEN WITHOUT CONSULTING
//
// A TUTOR OR CODE WRITTEN BY OTHER STUDENTS - Jiaying Lu
//

#include "defs.h"
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <signal.h>
#include <setjmp.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#define ERR_USAGE "USAGE: report [-k]\n"

int main(int argc, char *argv[]) {
    extern char *optarg;
    extern int optind;

    int start = -1;
    int sid = -1;        /* segment id of shared memory segment */
    int qid = -1;        /* message queue id */
    int semid = -1;      /* semaphore id */
    struct sembuf sb;       /* semaphore buffer */
    void * bit_map;
    int * perfect_num_arr;
    process_t * prc_arr;
    pid_t * manage_pid;
    process_t * stat;
    msg_t my_msg;
    int opt;
    int flag_k = 0;
    int i;
    long tested = 0, skipped = 0, found = 0;

    while ((opt = getopt(argc, argv, "k")) != -1) {
        switch (opt) {
            case 'k': 
                flag_k = 1;
                break;
            case '?': 
                fprintf(stderr, ERR_USAGE);
                return 1;
        }
    }

    /* create shared segment if necessary */
    if ((sid=shmget(KEY, (size_t)SHM_SIZE, IPC_CREAT |0660))== -1) {
        perror("manage.c -shmget");
        exit(1);
    }
    /* map it into our address space*/
    if ((bit_map=(shmat(sid,0,0))) == (void *)-1) {
        perror("manage.c -shmat");
        exit(1);
    }

    /* Ground data in bit map */
    perfect_num_arr = bit_map + BMAP_SIZE;
    prc_arr = (process_t*)(perfect_num_arr + MAX_PRC_NUM);
    manage_pid = (pid_t*)(prc_arr + MAX_PRC_NUM);
    stat = (process_t*)(manage_pid+1);

    /* Statistics jobs */
    // perfect nums
    printf("Perfect Number Found: ");
    for (i=0; i<MAX_PRC_NUM; i++) {
        if (perfect_num_arr[i] == -1) break;
        printf("%d ", perfect_num_arr[i]);
    }
    printf("\n");
    // compute processes
    for (i=0; i<MAX_PRC_NUM; i++) {
        if (prc_arr[i].pid == -1) continue;
        printf("pid(%d): found: %d, tested: %d, skipped: %d\n",\
                prc_arr[i].pid, prc_arr[i].found, prc_arr[i].tested, prc_arr[i].skipped);
        found += prc_arr[i].found;
        tested += prc_arr[i].tested;
        skipped += prc_arr[i].skipped;
    }
    // Statistics
    found += stat->found;
    tested += stat->tested;
    skipped += stat->skipped;
    printf("Statistics:\n");
    printf("Total found:   %-8ld\n", found);
    printf("Total tested:  %-8ld\n", tested);
    printf("Total skipped: %-8ld\n", skipped);

    /* Terminate manage process */
    if (flag_k) {
        kill(*manage_pid, SIGINT);
    }
    return 0;
}
