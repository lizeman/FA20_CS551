// THIS CODE IS MY OWN WORK, IT WAS WRITTEN WITHOUT CONSULTING
//
// A TUTOR OR CODE WRITTEN BY OTHER STUDENTS - Jiaying Lu
//

#include "ppmcvt.h"
#include "string.h"
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

int main (char argc, char *argv[]) {
    extern char *optarg;
    extern int optind;
    int opt, err = 0;
    char *in_fname = NULL;
    char *out_fname = NULL;
    
    while ((opt = getopt(argc, argv, "bg:i:r:smt:n:o:")) != -1) {
        switch (opt) {
            case 'o':
                out_fname = optarg;
                break;
            case '?':
                err = 1;
                break;
        }
    }

    if (out_fname == NULL) out_fname = DEFAULT_OUT_FNAME;
    printf ("[DEBUG] out_fname=%s\n", out_fname);

    printf("[DEBUG] optind=%d, argc=%d\n", optind, argc);
    if (optind + 1 == argc) {
        in_fname = argv[optind];
    } else if (optind == argc) {
        in_fname = DEFAULT_IN_FNAME;  //TODO: read from standard input
    } else {
        err = 1;
    }
    printf("[DEBUG] in_fname=%s\n", in_fname);

    if (err) {
        fprintf(stderr, USAGE);
	exit(1);
    }
}
