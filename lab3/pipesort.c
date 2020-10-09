#include <stdio.h> 
#include <stdlib.h>
#include <getopt.h>
#include <sys/types.h> 
#include <unistd.h> 

#define ERR_USAGE "USAGE: pipesort -n sorter_cnt -s short -l long\n"
#define ERR_OPTION "pipesort: Invalid option: -%c\n"
#define ERR_INVALID_ARG "pipsort: Invalid option argument: -%c %s \n"
#define ERR_INVALID_ARG_CHAR "pipsort: expected -s < -l, but violates when -s=%d, -l=%d\n"

int main(int argc, char *argv[]) 
{ 
    extern char *optarg;
    extern int optind;
    int min_char=-1, max_char=-1, pipe_count=-1;
    int opt;
    char * cptr;
    char buf;


    while ((opt = getopt(argc, argv, "n:s:l:")) != -1) {
        switch (opt) {
            case 'n':
                pipe_count = strtol(optarg, &cptr, 10);
                if (pipe_count < 0) {
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
    if (pipe_count == -1 || min_char == -1 || max_char == -1) {
        fprintf(stderr, ERR_USAGE);
        return 1;
    }
    if (min_char >= max_char) {
        fprintf(stderr, ERR_INVALID_ARG_CHAR, min_char, max_char);
        return 1;
    }
    printf("pipesort pipe_cnt=%d, short=%d, long=%d\n", pipe_count, min_char, max_char);

    printf("buf:\n");
    while (fgets(&buf, 2, stdin)) {
        printf("%c", buf);
    }
    printf("\n");
    printf("End fgets\n");
    if (ferror(stdin)) {
        perror("Stdin read error");
        exit(1);
    }
} 
