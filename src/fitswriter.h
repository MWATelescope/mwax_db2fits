/**
 * @file fitswriter.h
 * @author Greg Sleap
 * @date 15 May 2018
 * @brief This is the header for the code that handles writing fits files
 *
 */
#pragma once

#include "fitsio.h"

#define FITS_SIZE_CUTOFF_BYTES 200000

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
                       int polarisations, float int_time, float *buffer, uint64_t bytes);