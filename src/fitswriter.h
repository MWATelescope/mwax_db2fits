/**
 * @file fitswriter.h
 * @author Greg Sleap
 * @date 15 May 2018
 * @brief This is the header for the code that handles writing fits files
 *
 */
#ifndef XC_FITSWRITER_H_
#define XC_FITSWRITER_H_

#define PROJ_ID_LEN FLEN_VALUE

#include "fitsio.h"

/*typedef struct 
{
    long obsid;
    float inttime;
    char* project;
} metafits_info;*/

int open_fits(fitsfile **fptr, const char* filename);
int create_fits(fitsfile **fptr, const char* filename);
int close_fits(fitsfile **fptr);
int create_fits_imghdu(fitsfile *fptr, time_t unix_time, int unix_millisecond_time, int marker, int baselines, int fine_channels, 
                       int polarisations, float int_time, char *buffer, uint64_t bytes);

#endif /* XC_FITSWRITER_H_ */
