// THIS CODE IS MY OWN WORK, IT WAS WRITTEN WITHOUT CONSULTING
//
// A TUTOR OR CODE WRITTEN BY OTHER STUDENTS - Jiaying Lu
//
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

int do_parse(int sorter_count, int max_char, int min_char, FILE **fps) {
    int pipe_idx = 0;
    int char_cnt = 0;
    char * word = (char *) malloc((max_char+1) * sizeof(char));
    char buf[2];
    while (fgets(buf, 2, stdin)) {
        if (is_alphabetic_do_lower(buf)) {
            if (char_cnt < max_char) {
                word[char_cnt++] = buf[0];
            }
        } else {
            word[char_cnt] = '\0';
            if (char_cnt > min_char) {
                //printf("In parser(pid=%d) write %s into pipe#%d\n", getpid(), word, pipe_idx);
                fputs(word, fps[pipe_idx]);
                fputs("\n", fps[pipe_idx++]);
                pipe_idx %= sorter_count;
            }
            char_cnt = 0;
        }
    }
    if (char_cnt > min_char) {
        //printf("In parser(pid=%d) write %s into pipe#%d\n", getpid(), word, pipe_idx);
        fputs(word, fps[pipe_idx]);
        fputs("\n", fps[pipe_idx++]);
        pipe_idx %= sorter_count;
    }
    free(word);
    if (ferror(stdin)) {
        perror("Stdin read error");
        return 1;
    }
    return 0;
}

void clean_tail_break(char * word) {
    size_t len = strlen(word);
    if (word[len-1] == '\n') {
        word[len-1] = '\0';
    }
    return;
}

int do_merge(int sorter_count, int max_char, FILE **fps) {
    char ** word_cur;   // 2D: sorter_count * max_char+1
    //char word_out[max_char+1];
    char * word_out = (char *) malloc((max_char+1) * sizeof(char));
    unsigned long word_cnt = 0;
    unsigned int word_fp_end_flag = 0;
    int i;
    unsigned int merge_end_flag = 1;
    char * ret_fgets;

    word_cur = (char **) malloc(sorter_count * sizeof(char*));
    for (i = 0; i<sorter_count; i++) {
        word_cur[i] = (char *) malloc((max_char+1) * sizeof(char));
    }
    strcpy(word_out, "{");    // in ascii, { is behind z

    // Get the top-rank word_out
    for (i = 0; i < sorter_count; i++) {
        if (fgets(word_cur[i], max_char+1, fps[i]) == NULL) {  // cond: fp ends
            word_fp_end_flag |= (1 << i);
            strcpy(word_cur[i], "");
            continue;
        } else {
            clean_tail_break(word_cur[i]);
            //printf("First Merge(pid=%d) receieve word=%s from sorter#%d\n", getpid(), word_cur[i], i);
            if (strcmp(word_cur[i], word_out) < 0) strcpy(word_out, word_cur[i]);
        }
    }

    do {
        // Get word count
        for (i = 0; i < sorter_count; i++) {
            if ((word_fp_end_flag & (1 << i)) != 0) continue;  // word_cur[i] ended
            while (strcmp(word_cur[i], word_out) == 0) {
                word_cnt += 1;
                while ((ret_fgets=fgets(word_cur[i], max_char+1, fps[i])) != NULL) {
                     if (strcmp(word_cur[i], "\n") != 0) break;
                }
                if (ret_fgets == NULL) {
                    word_fp_end_flag |= (1 << i);
                    strcpy(word_cur[i], "");
                    break;   // break while strcmp
                } else {
                    clean_tail_break(word_cur[i]);
                }

                /*
                if (fgets(word_cur[i], max_char+1, fps[i]) == NULL) {  // check fp ends
                    word_fp_end_flag |= (1 << i);
                    strcpy(word_cur[i], "");
                    break;   // break while strcmp
                } else {
                    clean_tail_break(word_cur[i]);
                    //printf("Merge(pid=%d) receieve word=%s from sorter#%d\n", getpid(), word_cur[i], i);
                }
                */
            }  // End while strcmp
        }
        if (strcmp(word_out, "")) fprintf(stdout, "%-10lu%s\n", word_cnt, word_out);
        //fprintf(stdout, "%-10lu%s\n", word_cnt, word_out);
        // check whether end merge
        merge_end_flag = 1;
        for (i = 0; i < sorter_count; i++) {
            if ((word_fp_end_flag & (1 << i)) == 0) merge_end_flag &= 0;  // word_cur[i] ended
        }
        if (merge_end_flag == 1) break;   // break do-while

        // Get the top-rank word_out
        word_cnt = 0;
        strcpy(word_out, "{");    // in ascii, { is behind z
        for (i = 0; i < sorter_count; i++) {
            if ((word_fp_end_flag & (1 << i)) != 0) continue;  // word_cur[i] ended
            //if (strlen(word_cur[i]) <= 0) continue;
            if (strcmp(word_cur[i], word_out) < 0) strcpy(word_out, word_cur[i]);
        }
    } while (1);

    for (i = 0; i<sorter_count; i++) free(word_cur[i]);
    free(word_cur);
    return 0;
}

int main(int argc, char *argv[]) 
{ 
    extern char *optarg;
    extern int optind;
    int min_char=-1, max_char=-1, sorter_count=-1;
    int opt;
    char * cptr;
    int ret = 0;
    int i;

    int pids[3];    // 0: main, 1: parse, 2: merge
    int * pid_sorters;
    int whom;      // pid of dead children
    int status;    // child returns status

    int ** pipefd_parse;  // size: sorter_cnt * 2
    int ** pipefd_merge;  // size: sorter_cnt * 2

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
    //printf("pipesort sorter_count=%d, short=%d, long=%d\n", sorter_count, min_char, max_char);

    pids[0] = getpid();
    //printf("In Main process (pid=%d)\n", getpid());
    pid_sorters = (int *) malloc(sorter_count * sizeof(int));
    pipefd_parse = (int **) malloc(sorter_count * sizeof(int *));
    pipefd_merge = (int **) malloc(sorter_count * sizeof(int *));
    do {   // Tricky way to handle errors
        // Pipe between parser and sorter
        for (i = 0; i < sorter_count; i++) {
            pipefd_parse[i] = (int *) malloc(2 * sizeof(int));
            if (pipe(pipefd_parse[i]) == -1) {
                perror("Pipe between parser and sorter");
                ret = 1;
                break;
            }
            pipefd_merge[i] = (int *) malloc(2 * sizeof(int));
            if (pipe(pipefd_merge[i]) == -1) {
                perror("Pipe between merger and sorter");
                ret = 1;
                break;
            }
        }
    } while (0);
    // Clean Up
    if (ret != 0) {
        free(pid_sorters);
        for (i = 0; i < sorter_count; i++) {
            free(pipefd_parse[i]);
            free(pipefd_merge[i]);
        }
        free(pipefd_parse);
        free(pipefd_merge);
        return ret;
    }

    // Fork Parser and Merger
    for (int pidx=1; pidx<3; pidx++) {
        if ((pids[pidx]=fork()) == -1) {
            perror("Fork parse and merge process");
            exit(1);   // different with exit and return
        }
        if (pidx == 1 && pids[1] == 0) {   // inside Parse Process
            //printf("Parser Process(pid=%d) starts\n", getpid());
            int parser_ret = 0;
            FILE * fps[sorter_count];
            for (i = 0; i < sorter_count; i++) {
                close(pipefd_parse[i][0]);   // close read end for parser
                fps[i] = fdopen(pipefd_parse[i][1], "w");  // attach FILE* to write end for paser
                if (fps[i] == NULL) {
                    perror("OutPipe between parser and sorter - fdopen");
                    exit(1);
                }
                close(pipefd_merge[i][0]);   // close read end for merger
                close(pipefd_merge[i][1]);   // close write end for merger
            }
            parser_ret = do_parse(sorter_count, max_char, min_char, fps);
            for (i = 0; i < sorter_count; i++) {
                fclose(fps[i]);
                close(pipefd_parse[i][1]);   // close write end for parser
            }
            //printf("Parser Process(pid=%d) ends\n", getpid());
            exit(parser_ret);   // Parse Process exit
        } else if (pidx == 2 & pids[2] == 0) {   // insider Merge Process
            int merger_ret;
            FILE * fps[sorter_count];
            for (i = 0; i < sorter_count; i++) {
                close(pipefd_parse[i][0]);   // close read end for parser
                close(pipefd_parse[i][1]);   // close write end for parser
                fps[i] = fdopen(pipefd_merge[i][0], "r");  // attach FILE* to read end for merger
                if (fps[i] == NULL) {
                    perror("InPipe between merger and sorter - fdopen");
                    exit(1);
                }
                close(pipefd_merge[i][1]);   // close write end for merger
            }
            merger_ret = do_merge(sorter_count, max_char, fps);
            for (i = 0; i < sorter_count; i++) {
                fclose(fps[i]);
                close(pipefd_merge[i][0]);   // clser read end for merger
            }
            //printf("Merger Process(pid=%d) ends\n", getpid());
            exit(merger_ret);     // Merge Process exit
        }
    }

    // In Main Process. fork Sorter Processes
    for (i=0; i < sorter_count; i++) {
        if ((pid_sorters[i]=fork()) == -1) {
            perror("Fork sorter process");
            exit(1);
        }
        if (pid_sorters[i] == 0) {
            // This is sorter process
            //printf("In sorter(pid=%d) when i=%d\n", getpid(), i);
            for (int j = 0; j < sorter_count; j++) {
                close(pipefd_parse[j][1]);    // close write end for parser
                close(pipefd_merge[j][0]);    // close read end for merger
                if (i != j) {
                    close(pipefd_parse[j][0]);   // close sibling's read end for parser
                    close(pipefd_merge[j][1]);   // close sibling's write end for merger
                } 
            }
            dup2(pipefd_parse[i][0], STDIN_FILENO);
            dup2(pipefd_merge[i][1], STDOUT_FILENO);
            /*
            // debug
            char fname[16];
            sprintf(fname, "sorter_%d.out", i);
            execl("/usr/bin/sort", "sort", "-o", fname, NULL);
            // End debug
            */
            //fprintf(stderr, "In sorter(pid=%d) when i=%d, just before execl\n", getpid(), i);
            execl("/usr/bin/sort", "sort", (char*)NULL);
            close(pipefd_parse[i][0]);
            close(pipefd_merge[i][1]);
            //fprintf(stderr, "End sorter(pid=%d) when i=%d\n", getpid(), i);
            exit(0);
        }
    }

        
    for (i=0; i < sorter_count; i++) {
        close(pipefd_parse[i][0]);
        close(pipefd_parse[i][1]);
        close(pipefd_merge[i][0]);
        close(pipefd_merge[i][1]);
    }
    // Main Process Wait
    //for (i=0; i < 2 + sorter_count; i++) {
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
        //printf("In main process (pid=%d), wait after child (pid=%d) dead\n", getpid(), whom);
    }
    free(pid_sorters);
    for (i=0; i < sorter_count; i++) {
        free(pipefd_parse[i]);
        free(pipefd_merge[i]);
    }
    free(pipefd_parse);
    free(pipefd_merge);
    return ret;
} 
