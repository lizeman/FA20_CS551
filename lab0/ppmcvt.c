// THIS CODE IS MY OWN WORK, IT WAS WRITTEN WITHOUT CONSULTING
//
// A TUTOR OR CODE WRITTEN BY OTHER STUDENTS - Jiaying Lu
//

#include "pbm.h"
#include "ppmcvt.h"
#include "string.h"
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

//new functions return a properly initialized image struct of the appropriate type, with all necessary memory for the pixel array, pixmap, properly malloc'd
PPMImage * new_ppmimage(unsigned int width, unsigned int height, unsigned int max) {
    PPMImage *image = NULL;
    image = malloc(sizeof *image);
    if (image == NULL) {
        fprintf(stderr, ERR_MALLOC);
        exit(1);
    }
    image->width = width;
    image->height = height;
    image->max = max;
    for (int i=0; i<3; i++) {
        image->pixmap[i] = malloc(height * sizeof(unsigned int *));
        if (image->pixmap[i] == NULL) {
            fprintf(stderr, ERR_MALLOC);
            exit(1);
        }
        for (int h=0; h<height; h++) {
            image->pixmap[i][h] = malloc(width * sizeof(unsigned int));
            if (image->pixmap[i][h] == NULL) {
                fprintf(stderr, ERR_MALLOC);
                exit(1);
            }
        }
    }
    return image;
}

PGMImage * new_pgmimage( unsigned int width, unsigned int height, unsigned int max) {
    PGMImage *image = NULL;
    image = malloc(sizeof *image);
    if (image == NULL) {
        fprintf(stderr, ERR_MALLOC);
        exit(1);
    }
    image->width = width;
    image->height = height;
    image->max = max;
    image->pixmap = malloc(height * sizeof(unsigned int *));
    if (image->pixmap == NULL) {
        fprintf(stderr, ERR_MALLOC);
        exit(1);
    }
    for (int h=0; h<height; h++) {
        image->pixmap[h] = malloc(width * sizeof(unsigned int));
        if (image->pixmap[h] == NULL) {
            fprintf(stderr, ERR_MALLOC);
            exit(1);
        }
    }
    return image;
}

PBMImage * new_pbmimage( unsigned int width, unsigned int height ) {
    PBMImage *image = NULL;
    image = malloc(sizeof *image);
    if (image == NULL) {
        fprintf(stderr, ERR_MALLOC);
        exit(1);
    }
    image->width = width;
    image->height = height;
    image->pixmap = malloc(height * sizeof(unsigned int *));
    if (image->pixmap == NULL) {
        fprintf(stderr, ERR_MALLOC);
        exit(1);
    }
    for (int h=0; h<height; h++) {
        image->pixmap[h] = malloc(width * sizeof(unsigned int));
        if (image->pixmap[h] == NULL) {
            fprintf(stderr, ERR_MALLOC);
            exit(1);
        }
    }
    return image;
}

//del routines free ALL memory associated with image struct including the input image struct pointer
void del_ppmimage(PPMImage * image) {
    for (int i=0; i<3; i++) {
        for (int h=0; h<image->height; h++) {
            free(image->pixmap[i][h]);
        }
        free(image->pixmap[i]);
    }
    image = NULL;
}

void del_pgmimage( PGMImage * image) {
    for (int h=0; h<image->height; h++) {
        free(image->pixmap[h]);
    }
    free(image->pixmap);
    image = NULL;
}

void del_pbmimage( PBMImage * image) {
    for (int h=0; h<image->height; h++) {
        free(image->pixmap[h]);
    }
    free(image->pixmap);
    image = NULL;
}

// convertion functions
PBMImage * convert_ppm_to_pbm(PPMImage * ppm) {
    PBMImage * pbm = new_pbmimage(ppm->width, ppm->height);
    for (int h=0; h<ppm->height; h++) {
        for (int w=0; w<ppm->width; w++) {
            pbm->pixmap[h][w] = (ppm->pixmap[0][h][w] + ppm->pixmap[1][h][w] \
                    + ppm->pixmap[2][h][w]) / 3.0 < ppm->max / 2.0;
        }
    }
    return pbm;
}

PGMImage * convert_ppm_to_pgm(PPMImage * ppm, unsigned int pgm_max) {
    printf("[DEBUG] inside convert_ppm_to_pgm, pgm_max=%u\n", pgm_max);
    PGMImage * pgm = new_pgmimage(ppm->width, ppm->height, pgm_max);
    for (int h=0; h<ppm->height; h++) {
        for (int w=0; w<ppm->width; w++) {
            pgm->pixmap[h][w] = (ppm->pixmap[0][h][w] + ppm->pixmap[1][h][w] \
                    + ppm->pixmap[2][h][w]) / 3.0 / ppm->max * pgm->max;
        }
    }
    return pgm;
}

void sepia_transform_inplace(PPMImage * ppm) {
    unsigned int new_r, new_g, new_b;
    for (int h=0; h<ppm->height; h++) {
        for (int w=0; w<ppm->width; w++) {
            new_r = 0.393 * ppm->pixmap[0][h][w] + 0.769 * ppm->pixmap[1][h][w] \
                    + 0.189 * ppm->pixmap[2][h][w];
            new_g = 0.349 * ppm->pixmap[0][h][w] + 0.686 * ppm->pixmap[1][h][w] \
                    + 0.168 * ppm->pixmap[2][h][w];
            new_b = 0.272 * ppm->pixmap[0][h][w] + 0.534 * ppm->pixmap[1][h][w] \
                    + 0.131 * ppm->pixmap[2][h][w];
            ppm->pixmap[0][h][w] = min(new_r, ppm->max);
            ppm->pixmap[1][h][w] = min(new_g, ppm->max);
            ppm->pixmap[2][h][w] = min(new_b, ppm->max);
        }
    }
}


// argument parse
int main (char argc, char *argv[]) {
    extern char *optarg;
    extern int optind;
    int opt, err = 0;
    char *in_fname = NULL;
    char *out_fname = NULL;
    char conv = 'b';
    bool have_conv = false;
    long ret;
    char * cptr;
    PPMImage * ppm = NULL;
    PBMImage * pbm = NULL;
    PGMImage * pgm = NULL;
    
    while ((opt = getopt(argc, argv, "bg:i:r:smt:n:o:")) != -1) {
        switch (opt) {
            case 'b':
            case 's':
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
                conv = 'g';
                have_conv = true;
                ret = strtol(optarg, &cptr, 10);
                if (ret < 1 || ret > 65535) {
                    fprintf(stderr, ERR_INVALID_G, optarg);
                    exit(1);
                }
                break;
            case 'o':
                out_fname = optarg;
                break;
            case '?':
                err = 1;
                break;
        }
    }

    if (err) {
        fprintf(stderr, USAGE);
	exit(1);
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
        case 's':
            sepia_transform_inplace(ppm);
            write_ppmfile(ppm, out_fname);
        case '?':
            // should not touch this cond
            break;
    }
    del_ppmimage(ppm);
    return 0;
}
