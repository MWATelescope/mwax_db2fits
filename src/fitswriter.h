/*
 * fitswriter.h
 *
 *  Created on: 15-May-2018
 *      Author: Greg Sleap
 */
#ifndef XC_FITSWRITER_H_
#define XC_FITSWRITER_H_

#define PROJ_ID_LEN FLEN_VALUE

typedef struct 
{
    long obsid;
    float inttime;
    char* project;
} metafits_info;

int open_fits(fitsfile **fptr, const char* filename);
int read_metafits(fitsfile *fptr_metafits, metafits_info *mptr);
int create_fits(fitsfile **fptr, const char* filename, metafits_info *mptr);
int close_fits(fitsfile *fptr);

#endif /* XC_FITSWRITER_H_ */
