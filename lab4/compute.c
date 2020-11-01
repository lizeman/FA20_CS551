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

#define ERR_USAGE "USAGE: compute START\n"

void set_bit(int* bit_map, int k) {
    int bits = 8 * sizeof(int *);
    bit_map[k/bits] |= (1 << (k % bits));
}

int test_bit(int* bit_map, int k) {
    int bits = 8 * sizeof(int *);
    if ( (bit_map[k/bits] & (1 << (k%bits) )) != 0 ) return 1;  // k-th bit is 1
    else return 0;
}

void perfect(int start, void* bit_map, process_t* prc) {
    int i, sum;
    int n=start;

    while (1) {
        if (!test_bit(bit_map, n)) {  // n-th bit is 0
            sum=1;
            for (i=2;i<n;i++)
                if (!(n%i)) sum+=i;
            if (sum==n) {
                prc->found += 1;
                // TODO: add report msg sending
            }
            prc->tested += 1;
            set_bit(bit_map, n);
        } else {  // n-th bit is 1
            prc->skipped += 1;
        }
        n++;
        // n hit the end
        if (n >= 8 * BMAP_SIZE) break;
    }
    // TODO: exec "report -k"
    while (1) {
        ;  // wait
    }
}

jmp_buf jmpenv; /* environment saved by setjmp*/

int main(int argc, char *argv[]) {
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
    int my_pid = getpid();
    int my_prc_arr_idx;

    void quit();

    if (argc != 2) {
        fprintf(stderr, ERR_USAGE);
    }
    start = atoi(argv[1]);
    printf("Compute start=%d\n", start);

    /* create shared segment if necessary */
    if ((sid=shmget(KEY, (size_t)SHM_SIZE, IPC_CREAT |0660))== -1) {
        perror("compute.c -shmget");
        exit(1);
    }
    /* map it into our address space*/
    if ((bit_map=(shmat(sid,0,0))) == (void *)-1) {
        perror("compute.c -shmat");
        exit(1);
    }
    /* create queue if necessary */
    if ((qid=msgget(KEY,IPC_CREAT |0660))== -1) {
        perror("compute.c -msgget");
        exit(1);
    }
    /* get semaphore id*/
    if ((semid=semget(KEY,1 ,IPC_CREAT |0660))== -1) {
        perror("compute.c -semget");
        exit(1);
    }

    /* Ground data in bit map*/
    perfect_num_arr = bit_map + BMAP_SIZE;
    prc_arr = (process_t*)(perfect_num_arr + MAX_PRC_NUM);
    manage_pid = (pid_t*)(prc_arr + MAX_PRC_NUM);
    stat = (process_t*)(manage_pid+1);
    /* Register PID */
    my_msg.type = MSG_TYPE_REGISTER;
    my_msg.data = my_pid;
    msgsnd(qid, &my_msg, sizeof(my_msg.data), 0);

    /* set up for a lock operation after sending registration */
    sb.sem_op = -1;
    sb.sem_num = 0;  
    sb.sem_flg = 0;  
    if (semop(semid, &sb, 1) == -1) { /* will block if locked */
        perror("compute.c -sem lock");
        exit(1);
    }
    printf("compute(pid=%d) -After semaphore unlocked\n", my_pid);
    /* Scan prc_arr to locate my row */
    for (my_prc_arr_idx=0; my_prc_arr_idx < MAX_PRC_NUM; my_prc_arr_idx++) {
        if (my_pid == prc_arr[my_prc_arr_idx].pid) break;
    }
    printf("compute.c my_prc_arr_idx=%d, pid=%d\n", my_prc_arr_idx, prc_arr[my_prc_arr_idx].pid);
    if (my_prc_arr_idx >= MAX_PRC_NUM) {
        fprintf(stderr, "compute(pid=%d) can NOT locate index in bit_map\n", my_pid);
        return 1;
    }

    /* Quit() when tralvel from longjmp */
    if (setjmp(jmpenv)) {
        printf("compute(pid=%d) -Travel from longjmp\n", my_pid);
        prc_arr[my_prc_arr_idx].pid = -1;
        // add tested, skipped, found into stats
        stat->tested += prc_arr[my_prc_arr_idx].tested;
        stat->skipped += prc_arr[my_prc_arr_idx].skipped;
        stat->found += prc_arr[my_prc_arr_idx].found;
        return 0;
    }
    /* Catch signals: SIGINT, SIGHUP, SIGQUIT */
    signal(SIGINT, quit);
    signal(SIGHUP, quit);
    signal(SIGQUIT, quit);

    /* start compute perfect number */
    perfect(start, bit_map, prc_arr+my_prc_arr_idx);
    return 0;
}

void quit(signum)
    int signum;
{
    printf("compute.c -quit() signum=%d\n", signum);
    longjmp(jmpenv, 1);
}
