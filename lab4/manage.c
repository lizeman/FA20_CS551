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

jmp_buf jmpenv; /* environment saved by setjmp*/

void init_process_array(process_t * prc_arr) {
    for (int i=0; i<MAX_PRC_NUM; i++) {
        prc_arr[i].pid = -1;
    }
}

void register_compute_process(process_t * prc_arr, pid_t pid) {
    for (int i=0; i<MAX_PRC_NUM; i++) {
        if (prc_arr[i].pid == -1) {
            // initial a row in prc_arr
            prc_arr[i].pid = pid;
            prc_arr[i].tested = 0;
            prc_arr[i].skipped = 0;
            prc_arr[i].found = 0;
            return;
        }
    }
    // No available slot
    kill(pid, SIGINT);
}

void terminate_compute_process(process_t * prc_arr) {
    for (int i=0; i<MAX_PRC_NUM; i++) {
        if (prc_arr[i].pid == -1) kill(prc_arr[i].pid, SIGINT);
    }
}

int main() {
    int sid = -1;        /* segment id of shared memory segment */
    int qid = -1;        /* message queue id */
    int semid = -1;      /* semaphore id */
    struct sembuf sb;       /* semaphore buffer */
    void * bit_map = NULL;
    int * perfect_num_arr;
    process_t * prc_arr;
    pid_t * manage_pid;
    process_t * stat;    /* store stats on terminated `computes` */
    msg_t my_msg;
    int ret = 0;

    void quit();

    printf("SHM_SIZE=%ld\n", SHM_SIZE);
    /* create shared segment if necessary */
    if ((sid=shmget(KEY, (size_t)SHM_SIZE, IPC_CREAT |0660))== -1) {
        perror("manage.c -shmget");
        exit(1);
    }
    /* map it into our address space*/
    if ((bit_map=(shmat(sid,0,0))) == (void *)-1) {
        perror("manage.c -shmat");
        ret = 1;
    }
     /* create queue if necessary */
    if ((qid=msgget(KEY,IPC_CREAT |0660))== -1) {
        perror("manage.c -msgget");
        ret = 1;
    }
    /* get semaphore id*/
    if ((semid=semget(KEY,1 ,IPC_CREAT |0660))== -1) {
        perror("manage.c -semget");
        ret = 1;
    }
    
    /* Ground data in bit map*/
    perfect_num_arr = bit_map + BMAP_SIZE;
    prc_arr = (process_t*)(perfect_num_arr + MAX_PRC_NUM);
    manage_pid = (pid_t*)(prc_arr + MAX_PRC_NUM);
    stat = (process_t*)(manage_pid+1);
    //printf("bit_map=%p, perfect_num_arr=%p, sizeof(int)=%ld, prc_arr=%p\n", bit_map, perfect_num_arr, sizeof(int), prc_arr);
    //printf("sizeof(process_t)=%ld, manage_pid=%p, sizeof(pid_t)=%ld, stat=%p\n", sizeof(process_t), manage_pid, sizeof(pid_t), stat);
    *manage_pid = getpid();
    init_process_array(prc_arr);
    /* Cleanly deallocate all resources */
    if (setjmp(jmpenv)) {
        printf("manage.c -Travel from longjmp, start clear resources\n");
        /* Terminate compute processes*/
        terminate_compute_process(prc_arr);
        sleep(5);
        /* remove semaphore */
        if (semctl(semid, 0, IPC_RMID, 0)) {
            perror("manage.c -semctl IPC_RMID");
            ret = 1;
        }
        /* remove queue */
        if (msgctl(qid, IPC_RMID, 0) == -1) {
            perror("manage.c -msgctl IPC_RMID");
            ret = 1;
        }
        /* Unmap and deallocate the shared segment */
        if (shmdt(bit_map) == -1) {
            perror("manage.c -shmdt");
            ret = 1;
        }
        if (shmctl(sid, IPC_RMID, 0) == -1) {
            perror("manage.c -shmctl IPC_RMID");
            ret = 1;
        }
        return ret;
    }

    /* Catch signals: SIGINT, SIGHUP, SIGQUIT */
    signal(SIGINT, quit);
    signal(SIGHUP, quit);
    signal(SIGQUIT, quit);

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
            register_compute_process(prc_arr, my_msg.data);
            if (semop(semid, &sb, 1) == -1) { /* will unlock semaphore */
                perror("manage.c -sem unlock");
                exit(1);
            }
        }
    }

    quit(SIGQUIT);
    return ret;
}

void quit(signum)
    int signum;
{
    printf("manage.c -quit() signum=%d\n", signum);
    longjmp(jmpenv, 1);
}
