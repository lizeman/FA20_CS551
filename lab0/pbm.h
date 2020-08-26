//
//  pbm.h
//  ppmcvt
//
//  Created by Dorian Arnold on 8/5/20.
//  Copyright © 2020 Dorian Arnold. All rights reserved.
//

#ifndef pbm_h
#define pbm_h

typedef struct {
    unsigned int ** pixmap; //h x w (2-dimensional) bitmap array
    unsigned int height, width;
} PBMImage;

typedef struct {
    unsigned int ** pixmap; //h x w (2-dimensional) pixel array
    unsigned int height, width, max;
} PGMImage;

typedef struct {
    unsigned int ** pixmap[3]; //Three h x w (2-dimensional) pixel arrays, one array for R, G and B values, respectively.
    unsigned int height, width, max;
} PPMImage;

/** YOU MUST IMPLEMENT THE FOLLOWING NEW/DEL FUNCTIONS **/

//new functions return a properly initialized image struct of the appropriate type, with all necessary memory for the pixel array, pixmap, properly malloc'd
PPMImage * new_ppmimage( unsigned int width, unsigned int height, unsigned int max);
PGMImage * new_pgmimage( unsigned int width, unsigned int height, unsigned int max);
PBMImage * new_pbmimage( unsigned int width, unsigned int height );

//del routines free ALL memory associated with image struct including the input image struct pointer
void del_ppmimage( PPMImage * );
void del_pgmimage( PGMImage * );
void del_pbmimage( PBMImage * );

/** THESE FOLLOWING FUNCTIONS ARE IMPLEMENTED FOR YOU IN pmb.c **/
PPMImage * read_ppmfile( const char * filename );
void write_pbmfile( PBMImage *image, const char * filename );
void write_pgmfile( PGMImage *image, const char * filename );
void write_ppmfile( PPMImage *image, const char * filename );

#endif /* pbm_h */
