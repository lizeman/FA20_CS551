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

    return 0;
}
