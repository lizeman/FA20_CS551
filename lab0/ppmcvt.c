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
    image->width = width;
    image->height = height;
    image->max = max;
    for (int i=0; i<3; i++) {
        image->pixmap[i] = malloc(height * sizeof(unsigned int *));
        for (int h=0; h<height; h++) {
            image->pixmap[i][h] = malloc(width * sizeof(unsigned int));
        }
    }
    return image;
}

PGMImage * new_pgmimage( unsigned int width, unsigned int height, unsigned int max) {
    PGMImage *image = NULL;
    image = malloc(sizeof *image);
    image->width = width;
    image->height = height;
    image->max = max;
    image->pixmap = malloc(height * sizeof(unsigned int *));
    for (int h=0; h<height; h++) {
        image->pixmap[h] = malloc(width * sizeof(unsigned int));
    }
    return image;
}

PBMImage * new_pbmimage( unsigned int width, unsigned int height ) {
    PBMImage *image = NULL;
    image = malloc(sizeof *image);
    image->width = width;
    image->height = height;
    image->pixmap = malloc(height * sizeof(unsigned int *));
    for (int h=0; h<height; h++) {
        image->pixmap[h] = malloc(width * sizeof(unsigned int));
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


// argument parse
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

    if (out_fname == NULL) out_fname = DEFAULT_OUT_FNAME;   //TODO: output to stdout
    printf("[DEBUG] out_fname=%s\n", out_fname);

    printf("[DEBUG] optind=%d, argc=%d\n", optind, argc);
    if (optind + 1 == argc) {
        in_fname = argv[optind];
    } else if (optind == argc) {
        in_fname = DEFAULT_IN_FNAME;  //TODO: read from stdin
    } else {
        err = 1;
    }
    printf("[DEBUG] in_fname=%s\n", in_fname);

    if (err) {
        fprintf(stderr, USAGE);
	exit(1);
    }

    PPMImage * ppm = read_ppmfile(in_fname);
    printf("[DEBUG] ppm readed, h=%u, w=%u, maxval=%u\n", ppm->height, 
            ppm->width, ppm->max);
    del_ppmimage(ppm);
}
