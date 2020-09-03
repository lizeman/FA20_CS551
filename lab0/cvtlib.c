// THIS CODE IS MY OWN WORK, IT WAS WRITTEN WITHOUT CONSULTING
//
// A TUTOR OR CODE WRITTEN BY OTHER STUDENTS - Jiaying Lu
//

#include "pbm.h"
#include "ppmcvt.h"
#include "string.h"
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
    free(image);
    image = NULL;
}

void del_pgmimage( PGMImage * image) {
    for (int h=0; h<image->height; h++) {
        free(image->pixmap[h]);
    }
    free(image->pixmap);
    free(image);
    image = NULL;
}

void del_pbmimage( PBMImage * image) {
    for (int h=0; h<image->height; h++) {
        free(image->pixmap[h]);
    }
    free(image->pixmap);
    free(image);
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

void remove_RGB_channel_inplace(PPMImage * ppm, const char * channel) {
    int channel_idx;
    if (0 == strcmp(channel, RED))
        channel_idx = 0;
    else if (0 == strcmp(channel, GREEN))
        channel_idx = 1;
    else if (0 == strcmp(channel, BLUE))
        channel_idx = 2;
    else {
        fprintf(stderr, ERR_INVALID_CHANNEL, channel);
        exit(1);
    }
    for (int h=0; h<ppm->height; h++) {
        for (int w=0; w<ppm->width; w++) {
            ppm->pixmap[channel_idx][h][w] = 0;
        }
    }
}

void isolate_RGB_channel_inplace(PPMImage * ppm, const char * channel) {
    if (0 == strcmp(channel, RED)) {
        remove_RGB_channel_inplace(ppm, GREEN);
        remove_RGB_channel_inplace(ppm, BLUE);
    } else if (0 == strcmp(channel, GREEN)) {
        remove_RGB_channel_inplace(ppm, RED);
        remove_RGB_channel_inplace(ppm, BLUE);
    } else if (0 == strcmp(channel, BLUE)) {
        remove_RGB_channel_inplace(ppm, RED);
        remove_RGB_channel_inplace(ppm, GREEN);
    }
    else {
        fprintf(stderr, ERR_INVALID_CHANNEL, channel);
        exit(1);
    }
}

void mirror_vertically_inplace(PPMImage * ppm) {
    for (int h=0; h<ppm->height; h++) {
        for (int w=0; w<ppm->width/2; w++) {
            ppm->pixmap[0][h][ppm->width-w-1] = ppm->pixmap[0][h][w];
            ppm->pixmap[1][h][ppm->width-w-1] = ppm->pixmap[1][h][w];
            ppm->pixmap[2][h][ppm->width-w-1] = ppm->pixmap[2][h][w];
        }
    }
}

PPMImage * thumnail_ppm(PPMImage * ppm, unsigned int scale) {
    unsigned int width_thum = ppm->width/scale;
    unsigned int height_thum = ppm->height/scale;
    if (ppm->width % scale) width_thum++;
    if (ppm->height % scale) height_thum++;
    PPMImage * ppm_thum = new_ppmimage(width_thum, height_thum, ppm->max);
    for (int h=0; h<ppm_thum->height; h++) {
        for (int w=0; w<ppm_thum->width; w++) {
            ppm_thum->pixmap[0][h][w] = ppm->pixmap[0][h*scale][w*scale];
            ppm_thum->pixmap[1][h][w] = ppm->pixmap[1][h*scale][w*scale];
            ppm_thum->pixmap[2][h][w] = ppm->pixmap[2][h*scale][w*scale];
        }
    }
    return ppm_thum;
}

void nup_ppm_inplace(PPMImage * ppm, unsigned int scale) {
    unsigned int w_block = ppm->width/scale;
    unsigned int h_block = ppm->height/scale;
    if (ppm->width % scale) w_block++;
    if (ppm->height % scale) h_block++;
    for (int h=0; h<ppm->height; h++) {
        for (int w=0; w<ppm->width; w++) {
            if (h<h_block && w<w_block) {
                ppm->pixmap[0][h][w] = ppm->pixmap[0][h*scale][w*scale];
                ppm->pixmap[1][h][w] = ppm->pixmap[1][h*scale][w*scale];
                ppm->pixmap[2][h][w] = ppm->pixmap[2][h*scale][w*scale];
            } else {
                ppm->pixmap[0][h][w] = ppm->pixmap[0][h%h_block][w%w_block];
                ppm->pixmap[1][h][w] = ppm->pixmap[1][h%h_block][w%w_block];
                ppm->pixmap[2][h][w] = ppm->pixmap[2][h%h_block][w%w_block];
            }
        }
    }
}
