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
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#define ERR_USAGE "USAGE: compute START\n"

void perfect(int start) {
    int i,sum;

    int n=start;

    while (1) {
            sum=1;
            for (i=2;i<n;i++)
                    if (!(n%i)) sum+=i;
        
            if (sum==n) printf("%d is perfect\n",n);
            n++;
    }
}

int main(int argc, char *argv[]) {
    int start = -1;
    int sid;        /* segment id of shared memory segment */
    int qid;        /* message queue id */
    int semid;      /* semaphore id */
    struct sembuf sb;       /* semaphore buffer */
    void * bit_map;
    int * perfect_num_arr;
    process_t * prc_arr;
    pid_t * manage_pid;
    process_t * stat;
    msg_t my_msg;
    int my_pid = getpid();
    int my_prc_arr_idx;

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
        exit(2);
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
    /* Register PID */
    my_msg.type = MSG_TYPE_REGISTER;
    my_msg.data = my_pid;
    printf("compute.c pid=%d\n", my_msg.data);
    msgsnd(qid, &my_msg, sizeof(my_msg.data), 0);
    /* set up for a lock operation after sending registration */
    sb.sem_op = -1;
    sb.sem_num = 0;  
    sb.sem_flg = 0;  
    if (semop(semid, &sb, 1) == -1) { /* will block if locked */
        perror("compute.c -sem lock");
        exit(1);
    }
    printf("compute.c -After semaphore unlocked\n");
    /* Scan prc_arr to locate my row */
    for (my_prc_arr_idx=0; my_prc_arr_idx < MAX_PRC_NUM; my_prc_arr_idx++) {
        if (my_pid == prc_arr[my_prc_arr_idx].pid) break;
    }
    printf("compute.c my_prc_arr_idx=%d, pid=%d\n", my_prc_arr_idx, prc_arr[my_prc_arr_idx].pid);

    return 0;
}
