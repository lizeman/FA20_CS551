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

int main() {
    int sid;        /* segment id of shared memory segment */
    int qid;        /* message queue id */
    int semid;      /* semaphore id */
    struct sembuf sb;       /* semaphore buffer */
    void * bit_map;
    int * perfect_num_arr;
    process_t * prc_arr;
    int prc_registered = 0;
    pid_t * manage_pid;
    process_t * stat;
    msg_t my_msg;

    printf("SHM_SIZE=%ld\n", SHM_SIZE);
    /* create shared segment if necessary */
    if ((sid=shmget(KEY, (size_t)SHM_SIZE, IPC_CREAT |0660))== -1) {
        perror("manage.c -shmget");
        exit(1);
    }
    /* map it into our address space*/
    if ((bit_map=(shmat(sid,0,0))) == (void *)-1) {
        perror("manage.c -shmat");
        exit(2);
    }
     /* create queue if necessary */
    if ((qid=msgget(KEY,IPC_CREAT |0660))== -1) {
        perror("manage.c -msgget");
        exit(1);
    }
    /* get semaphore id*/
    if ((semid=semget(KEY,1 ,IPC_CREAT |0660))== -1) {
        perror("manage.c -semget");
        exit(1);
    }

    /* Ground data in bit map*/
    perfect_num_arr = bit_map + BMAP_SIZE;
    prc_arr = (process_t*)(perfect_num_arr + MAX_PRC_NUM);
    manage_pid = (pid_t*)(prc_arr + MAX_PRC_NUM);
    stat = (process_t*)(manage_pid+1);
    printf("bit_map=%p, perfect_num_arr=%p, sizeof(int)=%ld, prc_arr=%p\n", bit_map, perfect_num_arr, sizeof(int), prc_arr);
    printf("sizeof(process_t)=%ld, manage_pid=%p, sizeof(pid_t)=%ld, stat=%p\n", sizeof(process_t), manage_pid, sizeof(pid_t), stat);
    *manage_pid = getpid();
    printf("manage pid=%d\n", *manage_pid);

    /* set up for a lock operation after sending registration */
    sb.sem_op = 1;
    sb.sem_num = 0;  
    sb.sem_flg = 0;  
    /* Do manage job until recieve "report -k"*/
    while (1) {
        msgrcv(qid, &my_msg, sizeof(my_msg.data), 0, 0);
        if (my_msg.type == MSG_TYPE_REGISTER) {
            printf("manage.c receieve msg: type=%ld, data=%d\n", my_msg.type, my_msg.data);
            // initial a row in prc_arr
            prc_arr[prc_registered].pid = my_msg.data;
            prc_arr[prc_registered].tested = 0;
            prc_arr[prc_registered].skipped = 0;
            prc_arr[prc_registered].found = 0;
            prc_registered++;
            if (semop(semid, &sb, 1) == -1) { /* will unlock semaphore */
                perror("manage.c -sem unlock");
                exit(1);
            }
            break;
        }
    }

    //TODO: ensure release every resources; catch ctrl-c, ..
    /* remove semaphore */
    if (semctl(semid, 0, IPC_RMID, 0)) {
        perror("manage.c -semctl IPC_RMID");
        exit(3);
    }
    /* remove queue */
    if (msgctl(qid, IPC_RMID, 0) == -1) {
        perror("manage.c -msgctl IPC_RMID");
        exit(3);
    }
    /* Unmap and deallocate the shared segment */
    if (shmdt(bit_map) == -1) {
        perror("manage.c -shmdt");
        exit(3);
    }
    if (shmctl(sid, IPC_RMID, 0) == -1) {
        perror("manage.c -shmctl IPC_RMID");
        exit(3);
    }

    return 0;
}
