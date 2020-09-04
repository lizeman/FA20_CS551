// THIS CODE IS MY OWN WORK, IT WAS WRITTEN WITHOUT CONSULTING
//
// A TUTOR OR CODE WRITTEN BY OTHER STUDENTS - Jiaying Lu
//

#include "ppmcvt.h"
#include "string.h"
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include "pbm.h"

// argument parse
int main (char argc, char *argv[]) {
    extern char *optarg;
    extern int optind;
    int opt;
    char * in_fname = NULL;
    char * out_fname = NULL;
    char * channel = NULL;
    char conv = 'b';
    bool have_conv = false;
    long ret;
    char * cptr;
    PPMImage * ppm = NULL;
    PPMImage * ppm_conv = NULL;
    PBMImage * pbm = NULL;
    PGMImage * pgm = NULL;
    
    while ((opt = getopt(argc, argv, "bg:i:r:smt:n:o:")) != -1) {
        switch (opt) {
            case 'b':
            case 's':
            case 'm':
                if (have_conv) {
                    fprintf(stderr, ERR_MULTI_CONV);
                    exit(1);
                }
                conv = opt;
                have_conv = true;
                break;
            case 'g':
                if (have_conv) {
                    fprintf(stderr, ERR_MULTI_CONV);
                    exit(1);
                }
                conv = opt;
                have_conv = true;
                ret = strtol(optarg, &cptr, 10);
                if (ret < 1 || ret > 65535) {
                    fprintf(stderr, ERR_INVALID_G, optarg);
                    exit(1);
                }
                break;
            case 'i':
            case 'r':
                if (have_conv) {
                    fprintf(stderr, ERR_MULTI_CONV);
                    exit(1);
                }
                conv = opt;
                have_conv = true;
                channel = optarg;
                if (channel == NULL || \
                        (strcmp(channel, RED) && strcmp(channel, GREEN) && strcmp(channel, BLUE))) {
                    fprintf(stderr, ERR_INVALID_CHANNEL, channel);
                    exit(1);
                }
                break;
            case 't':
            case 'n':
                if (have_conv) {
                    fprintf(stderr, ERR_MULTI_CONV);
                    exit(1);
                }
                conv = opt;
                have_conv = true;
                ret = strtol(optarg, &cptr, 10);
                if (ret < 1 || ret > 8) {
                    fprintf(stderr, ERR_INVALID_SCALE, optarg);
                    exit(1);
                }
                break;
            case 'o':
                out_fname = optarg;
                break;
            case '?':
                /**
                if (optopt == 'o')
                    fprintf(stderr, ERR_NO_OUT);
                else
                    fprintf(stderr, USAGE);
                **/
                exit(1);
                break;
        }
    }

    // printf("[DEBUG] optind=%d, argc=%d\n", optind, argc);
    if (optind + 1 == argc) {
        in_fname = argv[optind];
    } else {
        fprintf(stderr, ERR_NO_IN);
        exit(1);
    }
    if (out_fname == NULL) {
        fprintf(stderr, ERR_NO_OUT);
        exit(1);
    }

    ppm = read_ppmfile(in_fname);
    switch (conv) {
        case 'b':
            pbm = convert_ppm_to_pbm(ppm);
            write_pbmfile(pbm, out_fname);
            del_pbmimage(pbm);
            break;
        case 'g':
            pgm = convert_ppm_to_pgm(ppm, ret);
            write_pgmfile(pgm, out_fname);
            del_pgmimage(pgm);
            break;
        case 'i':
            isolate_RGB_channel_inplace(ppm, channel);
            write_ppmfile(ppm, out_fname);
            break;
        case 'r':
            remove_RGB_channel_inplace(ppm, channel);
            write_ppmfile(ppm, out_fname);
            break;
        case 's':
            sepia_transform_inplace(ppm);
            write_ppmfile(ppm, out_fname);
            break;
        case 'm':
            mirror_vertically_inplace(ppm);
            write_ppmfile(ppm, out_fname);
            break;
        case 't':
            ppm_conv = thumnail_ppm(ppm, ret);
            write_ppmfile(ppm_conv, out_fname);
            del_ppmimage(ppm_conv);
            break;
        case 'n':
            nup_ppm_inplace(ppm, ret);
            write_ppmfile(ppm, out_fname);
            break;
        case '?':
            // should not touch this cond
            break;
    }
    del_ppmimage(ppm);
    return 0;
}
