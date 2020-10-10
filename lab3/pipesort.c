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
    char * word;

    int pids[3];    // 0: main, 1: parse, 2: merge
    int * pid_sorters;
    int whom;      // pid of dead children
    int status;    // child returns status

    int pipefd_parse[2];  //TODO: consider dynamically create pipes

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
    pid_sorters = malloc(sorter_count * sizeof(int));
    printf("In Main process (pid=%d)\n", getpid());
    // Pipe between parser and sorter
    if (pipe(pipefd_parse) == -1) {
        perror("Pipe between parser and sorter");
        exit(1);
    }

    // Start Parsing inside Parse Process
    if ((pids[1]=fork()) == -1) {
        perror("Fork parse process");
        exit(1);   // different with exit and return
    }
    if (pids[1] == 0) {
        close(pipefd_parse[0]);
        FILE * fp = fdopen(pipefd_parse[1], "w");
        if (fp == NULL) {
            perror("OutPipe between parser and sorter - fdopen");
            exit(1);
        }
        printf("In parse process (pid=%d)\n", getpid());
        word = malloc((max_char+1) * sizeof(char));
        word_cnt = 0;
        printf("Parsing Result:\n");
        while (fgets(&buf, 2, stdin)) {
            if (is_alphabetic_do_lower(&buf)) {
                if (word_cnt < max_char) {
                    word[word_cnt++] = buf;
                }
            } else {
                word[word_cnt] = '\0';
                if (word_cnt > min_char) {
                    printf("write %s into pipe\n", word);
                    fputs(word, fp);
                    fputs("\n", fp);
                }
                word_cnt = 0;
            }
        }
        if (word_cnt > min_char) {
            printf("write %s into pipe\n", word);
            fputs(word, fp);
            fputs("\n", fp);
        }
        printf("End Parsing\n");
        if (ferror(stdin)) {
            perror("Stdin read error");
            exit(1);
        }
        free(word);
        fclose(fp);
        close(pipefd_parse[1]);
        exit(0);   // Parse Process exit
    } else {   // Main Process
        // Sorter Processes
        for (int i=0; i < sorter_count; i++) {
            if ((pid_sorters[i]=fork()) == -1) {
                perror("Fork sorter process");
                exit(1);
            }
            if (pid_sorters[i] == 0) {
                // This is sorter process
                close(pipefd_parse[1]);
                printf("In sorter(pid=%d)\n", getpid());
                dup2(pipefd_parse[0], STDIN_FILENO);
                execl("/usr/bin/sort", "sort", (char*)NULL);
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
                close(pipefd_parse[0]);
                exit(0);
            }
        }
    }

        
    close(pipefd_parse[0]);
    close(pipefd_parse[1]);
    // Main Process Wait
    //for (int i=0; i < 2 + sorter_count
    for (int i=0; i < 1 + sorter_count; i++) {
        if ((whom=wait(&status)) == -1) {
            perror("Main wait");
            exit(1);
        }
        if (status != 0) {
            fprintf(stderr, "Child Process[pid=%d] returned %d, exit\n", whom, status);
            free(pid_sorters);
            exit(1);
        }
        printf("In main process (pid=%d), wait after child (pid=%d) dead\n", getpid(), whom);
    }
    free(pid_sorters);
    return 0;
} 
