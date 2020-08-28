// THIS CODE IS MY OWN WORK, IT WAS WRITTEN WITHOUT CONSULTING
// 
// A TUTOR OR CODE WRITTEN BY OTHER STUDENTS - Jiaying Lu
//

#ifndef ppmcvt_h
#define ppmcvt_h

#define USAGE "Usage: ppmcvt [-bgirsmtno] [FILE]\n"
#define ERR_NO_IN "Error: NO input file specified\n"
#define ERR_NO_OUT "Error: NO output file specified\n"
#define ERR_MALLOC "Error: Memory allocation failed\n"
#define ERR_MULTI_CONV "Error: Multiple transformations specified\n"
#define ERR_INVALID_G "Error: Invalid max grayscale pixel value: %s; must be less than 65,536\n"

#include "pbm.h"

typedef enum {false, true} bool;

#define max(a,b)             \
({                           \
    __typeof__ (a) _a = (a); \
    __typeof__ (b) _b = (b); \
    _a > _b ? _a : _b;       \
})

#define min(a,b)             \
({                           \
    __typeof__ (a) _a = (a); \
    __typeof__ (b) _b = (b); \
    _a < _b ? _a : _b;       \
})

// convertion functions
PBMImage * convert_ppm_to_pbm(PPMImage *);
PGMImage * convert_ppm_to_pgm(PPMImage *, unsigned int);
void sepia_transform_inplace(PPMImage *);

#endif /* ppmcvt_h */
