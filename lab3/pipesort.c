#include <stdio.h> 
#include <stdlib.h>
#include <getopt.h>
#include <sys/types.h> 
#include <unistd.h> 
#include <sys/wait.h>
#include <string.h>

#define ERR_USAGE "USAGE: pipesort -n sorter_cnt -s short -l long\n"
#define ERR_OPTION "pipesort: Invalid option: -%c\n"
#define ERR_INVALID_ARG "pipsort: Invalid option argument: -%c %s \n"
#define ERR_INVALID_ARG_CHAR "pipsort: expected -s < -l, but violates when -s=%d, -l=%d\n"


int is_alphabetic_do_lower(char * pc) {
    if (*pc >= 97 && *pc <= 122) {
        return 1;
    } else if (*pc >= 65 && *pc <= 90) {
        *pc = *pc - 65 + 97;
        return 1;
    } else {
        return 0;
    }
}

int main(int argc, char *argv[]) 
{ 
    extern char *optarg;
    extern int optind;
    int min_char=-1, max_char=-1, sorter_count=-1;
    int opt;
    char * cptr;
    char buf;
    int word_cnt;
    int ret = 0;
    int i;

    int pids[3];    // 0: main, 1: parse, 2: merge
    int * pid_sorters;
    int whom;      // pid of dead children
    int status;    // child returns status

    int ** pipefd_parse;  // size: sorter_cnt * 2

    while ((opt = getopt(argc, argv, "n:s:l:")) != -1) {
        switch (opt) {
            case 'n':
                sorter_count = strtol(optarg, &cptr, 10);
                if (sorter_count < 0) {
                    fprintf(stderr, ERR_INVALID_ARG, opt, optarg);
                    exit(1);
                }
                break;
            case 's':
                min_char = strtol(optarg, &cptr, 10);
                if (min_char < 0) {
                    fprintf(stderr, ERR_INVALID_ARG, opt, optarg);
                    exit(1);
                }
                break;
            case 'l':
                max_char = strtol(optarg, &cptr, 10);
                if (max_char < 0) {
                    fprintf(stderr, ERR_INVALID_ARG, opt, optarg);
                    exit(1);
                }
                break;
            case '?':
                fprintf(stderr, ERR_OPTION, opt);
                return 1;
        }
    }
    // Argument check
    if (sorter_count == -1 || min_char == -1 || max_char == -1) {
        fprintf(stderr, ERR_USAGE);
        return 1;
    }
    if (min_char >= max_char) {
        fprintf(stderr, ERR_INVALID_ARG_CHAR, min_char, max_char);
        return 1;
    }
    printf("pipesort sorter_count=%d, short=%d, long=%d\n", sorter_count, min_char, max_char);

    pids[0] = getpid();
    pid_sorters = (int *) malloc(sorter_count * sizeof(int));
    printf("In Main process (pid=%d)\n", getpid());
    // Pipe between parser and sorter
    pipefd_parse = (int **) malloc(sorter_count * sizeof(int *));
    for (i = 0; i < sorter_count; i++) {
        pipefd_parse[i] = (int *) malloc(2 * sizeof(int));
        if (pipe(pipefd_parse[i]) == -1) {
            perror("Pipe between parser and sorter");
            exit(1);
        }
    }

    // Start Parsing
    if ((pids[1]=fork()) == -1) {
        perror("Fork parse process");
        exit(1);   // different with exit and return
    }
    if (pids[1] == 0) {   // inside Parse Process
        char word[max_char+1];
        FILE * fps[sorter_count];
        int pipe_idx = 0;
        for (i = 0; i < sorter_count; i++) {
            close(pipefd_parse[i][0]);
            fps[i] = fdopen(pipefd_parse[i][1], "w");
            if (fps[i] == NULL) {
                perror("OutPipe between parser and sorter - fdopen");
                exit(1);
            }
        }
        printf("In parse process (pid=%d)\n", getpid());
        word_cnt = 0;
        while (fgets(&buf, 2, stdin)) {
            if (is_alphabetic_do_lower(&buf)) {
                if (word_cnt < max_char) {
                    word[word_cnt++] = buf;
                }
            } else {
                word[word_cnt] = '\0';
                if (word_cnt > min_char) {
                    printf("write %s into pipe\n", word);
                    fputs(word, fps[pipe_idx]);
                    fputs("\n", fps[pipe_idx++]);
                    pipe_idx %= sorter_count;
                }
                word_cnt = 0;
            }
        }
        if (word_cnt > min_char) {
            printf("write %s into pipe\n", word);
            fputs(word, fps[pipe_idx]);
            fputs("\n", fps[pipe_idx++]);
            pipe_idx %= sorter_count;
        }
        printf("End Parsing\n");
        if (ferror(stdin)) {
            perror("Stdin read error");
            exit(1);
        }
        for (i = 0; i < sorter_count; i++) {
            fclose(fps[i]);
            close(pipefd_parse[i][1]);
        }
        exit(0);   // Parse Process exit
    } else {   // Main Process
        // Sorter Processes
        for (i=0; i < sorter_count; i++) {
            if ((pid_sorters[i]=fork()) == -1) {
                perror("Fork sorter process");
                exit(1);
            }
            if (pid_sorters[i] == 0) {
                // This is sorter process
                printf("In sorter(pid=%d) when i=%d\n", getpid(), i);
                for (int j = 0; j < sorter_count; j++) {
                    close(pipefd_parse[j][1]);
                    if (i != j) {
                        close(pipefd_parse[j][0]);
                    } else {
                        dup2(pipefd_parse[j][0], STDIN_FILENO);
                        // debug
                        char fname[16];
                        sprintf(fname, "sorter_%d.out", i);
                        execl("/usr/bin/sort", "sort", "-o", fname, NULL);
                        // End debug
                        //execl("/usr/bin/sort", "sort", (char*)NULL);
                    }
                }
                /*
                FILE * fp = fdopen(pipefd_parse[0], "r");
                if (fp == NULL) {
                    perror("InPipe between parser and sorter - fdopen");
                    exit(1);
                }
                word = malloc((max_char+1) * sizeof(char));
                while (fgets(word, sizeof(word), fp)) {
                    printf("In sorter(pid=%d) receieve word=%s\n", getpid(), word);
                }
                free(word);
                fclose(fp);
                */
                // close(pipefd_parse[i][0]);  // Seems no need
                printf("End sorter(pid=%d) when i=%d\n", getpid(), i);
                exit(0);
            }
        }
    }

        
    for (i=0; i < sorter_count; i++) {
        close(pipefd_parse[i][0]);
        close(pipefd_parse[i][1]);
    }
    // Main Process Wait
    //for (i=0; i < 2 + sorter_count
    for (i=0; i < 1 + sorter_count; i++) {
        if ((whom=wait(&status)) == -1) {
            perror("Main wait");
            ret = 1;
            break;
        }
        if (status != 0) {
            fprintf(stderr, "Child Process[pid=%d] returned %d, exit\n", whom, status);
            ret = 1;
            break;
        }
        printf("In main process (pid=%d), wait after child (pid=%d) dead\n", getpid(), whom);
    }
    free(pid_sorters);
    for (i=0; i < sorter_count; i++) free(pipefd_parse[i]);
    free(pipefd_parse);
    return ret;
} 
