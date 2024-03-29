/**
 * @file fitswriter.h
 * @author Greg Sleap
 * @date 15 May 2018
 * @brief This is the header for the code that handles writing fits files
 *
 */
#pragma once

#include "fitsio.h"
#include "dada_client.h"

// Keys and some hard coded values for the 1st HDU of the fits file produced
#define MWA_FITS_KEY_SIMPLE "SIMPLE"
#define MWA_FITS_VALUE_SIMPLE TRUE
#define MWA_FITS_KEY_BITPIX "BITPIX"
#define MWA_FITS_VALUE_BITPIX 8
#define MWA_FITS_KEY_NAXIS "NAXIS"
#define MWA_FITS_VALUE_NAXIS 0
#define MWA_FITS_KEY_TIME "TIME"
#define MWA_FITS_KEY_MILLITIM "MILLITIM"
#define MWA_FITS_KEY_INTTIME "INTTIME"
#define MWA_FITS_KEY_NINPUTS "NINPUTS"
#define MWA_FITS_KEY_FINECHAN "FINECHAN"
#define MWA_FITS_KEY_NFINECHS "NFINECHS"
#define MWA_FITS_KEY_MARKER "MARKER"
#define MWA_FITS_KEY_PROJID "PROJID"
#define MWA_FITS_KEY_OBSID "OBSID"
#define MWA_FITS_KEY_CORR_VER "CORR_VER"
#define MWA_FITS_VALUE_CORR_VER 2
#define MWA_FITS_KEY_CORR_HOST "CORRHOST"
#define MWA_FITS_KEY_CORR_CHAN "CORRCHAN"
#define MWA_FITS_KEY_MC_IP "MC_IP"
#define MWA_FITS_KEY_MC_PORT "MC_PORT"
#define MWA_FITS_KEY_MWAX_U2S_VERSION "U2S_VER"
#define MWA_FITS_KEY_MWAX_DB2CORRELATE2DB_VERSION "CBF_VER"
#define MWA_FITS_KEY_MWAX_DB2FITS_VERSION "DB2F_VER"

int open_fits(dada_client_t *client, fitsfile **fptr, const char *filename);
int create_fits(dada_client_t *client, fitsfile **fptr, const char *filename);
int close_fits(dada_client_t *client, fitsfile **fptr, int fits_is_good);
int create_fits_visibilities_imghdu(dada_client_t *client, fitsfile *fptr, time_t unix_time, int unix_millisecond_time,
                                    int marker, int baselines, int fine_channels, int polarisations, float *buffer, uint64_t bytes);
int create_fits_weights_imghdu(dada_client_t *client, fitsfile *fptr, time_t unix_time, int unix_millisecond_time,
                               int marker, int baselines, int polarisations, float *buffer, uint64_t bytes);
