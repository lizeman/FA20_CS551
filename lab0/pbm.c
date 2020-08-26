//
//  pbm.c
//  ppmcvt
//
//  Created by Dorian Arnold on 8/5/20.
//  Copyright © 2020 Dorian Arnold. All rights reserved.
//

#include "pbm.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

PPMImage * read_ppmfile( const char * infilename ) {
    FILE * infile;
    char intype[3];
    int w, width, h, height, maxval;
    
    if( (infile = fopen( infilename, "r" )) == NULL ) {
        perror("fopen()");
        exit(-1);
    }
    
    if( fscanf( infile, "%s %d %d %d", intype, &width, &height, &maxval ) == EOF ) {
        fprintf(stderr, "Error reading input file.\n");
        exit(-1);
    }

    if ( strcmp(intype, "P3") ) {
        fprintf(stderr, "Wrong input file type! Magic number, %s, should be 'P1'\n",
                intype );
        exit(-1);
    }
    
    PPMImage * ppm = new_ppmimage( width, height, maxval );
    
    for( h=0; h<height; h++ ){
        for ( w=0; w<width; w++ ){
            if( fscanf( infile, "%u %u %u",
                        &ppm->pixmap[0][h][w],
                        &ppm->pixmap[1][h][w],
                        &ppm->pixmap[2][h][w] )
                        == EOF ) {
                fprintf(stderr, "Error reading input file.\n");
                        exit(-1);
            }
        }
    }
    
    fclose( infile );
    return ppm;
}

void write_ppmfile( PPMImage *ppm, const char *filename ){
    FILE * outfile;
    
    if( (outfile = fopen( filename, "w" )) == NULL ) {
        perror("fopen()");
        exit( -1 );
     }

    //write pbm header
    fprintf(outfile, "P3\n%d %d\n%d\n", ppm->width, ppm->height, ppm->max);
    
    //read ppm and write pbm file simultaneously
    for( int h=0; h<ppm->height; h++ ){
        for ( int w=0; w<ppm->width; w++ ){
            fprintf(outfile, "%d %d %d ",
                    ppm->pixmap[0][h][w],
                    ppm->pixmap[1][h][w],
                    ppm->pixmap[2][h][w] );
        }
            
        fprintf(outfile, "\n"); //newline after each row
    }
    
    fclose( outfile );
}

void write_pbmfile( PBMImage *pbm, const char * filename ) {
    FILE * outfile;
    
    if( (outfile = fopen( filename, "w" )) == NULL ) {
        perror("fopen()");
        exit( -1 );
     }

    //write pbm header
    fprintf(outfile, "P1\n%d %d\n", pbm->width, pbm->height);
    
    //read ppm and write pbm file simultaneously
    for( int h=0; h<pbm->height; h++ ){
        for ( int w=0; w<pbm->width; w++ ){
            fprintf(outfile, "%d", pbm->pixmap[h][w] );
        }
            
        fprintf(outfile, "\n"); //newline after each row
    }
    
    fclose( outfile );
}

void write_pgmfile( PGMImage *pgm, const char * filename ){
    FILE * outfile;
    
    if( (outfile = fopen( filename, "w" )) == NULL ) {
        perror("fopen()");
        exit( -1 );
     }

    //write pbm header
    fprintf(outfile, "P2\n%d %d\n%d\n", pgm->width, pgm->height, pgm->max);
    
    //read ppm and write pbm file simultaneously
    for( int h=0; h<pgm->height; h++ ){
        for ( int w=0; w<pgm->width; w++ ){
            fprintf(outfile, "%d ", pgm->pixmap[h][w] );
        }
            
        fprintf(outfile, "\n"); //newline after each row
    }
    
    fclose( outfile );
}
