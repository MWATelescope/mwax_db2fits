/*
 * fitswriter.h
 *
 *  Created on: 15-May-2018
 *      Author: Greg Sleap
 */
#ifndef XC_FITSWRITER_H_
#define XC_FITSWRITER_H_

#define PROJ_ID_LEN FLEN_VALUE

#include "fitsio.h"

typedef struct 
{
    long obsid;
    float inttime;
    char* project;
} metafits_info;

int open_fits(fitsfile **fptr, const char* filename);
int create_fits(fitsfile **fptr, const char* filename);
int close_fits(fitsfile *fptr);
int create_fits_imghdu(fitsfile *fptr, int baselines, int fine_channels, int polarisations, void *buffer, uint64_t bytes);

#endif /* XC_FITSWRITER_H_ */
