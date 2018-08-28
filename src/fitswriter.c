/**
 * @file fitswriter.c
 * @author Greg Sleap
 * @date 15 May 2018
 * @brief This is the code that handles writing fits files
 *
 */
#include <assert.h>
#include <errno.h>
#include <fitsio.h>
#include <stdio.h>
#include <string.h>

#include "global.h"
#include "fitswriter.h"
#include "utils.h"
#include "fitswriter.h"
#include "multilog.h"

/**
 * 
 *  @brief Creates a blank new fits file called 'filename' and populates it with data from the psrdada header.
 *  @param[in] client A pointer to the dada_client_t object.
 *  @param[out] fptr pointer to the pointer of the fitsfile created.
 *  @param[in] filename Full path and name of the fits file to create.
 *  @returns EXIT_SUCCESS on success, or -1 if there was an error.
 */
int create_fits(dada_client_t *client, fitsfile **fptr, const char *filename)
{
  assert(client != 0);
  dada_db_s* ctx = (dada_db_s*) client->context;

  assert(ctx->log != 0);
  multilog_t *log = (multilog_t *) client->log;  
  
  int status = 0;
      
  multilog(log, LOG_INFO, "create_fits(): Creating new fits file %s...\n", filename);

  // So CFITSIO overwrites the file, we should prefix the filename with !
  int len = strlen(filename)+2;
  char cfitsio_filename[len];
  sprintf(cfitsio_filename, "!%s", filename);

  // Create a new blank fits file
  if (fits_create_file(fptr, cfitsio_filename, &status))
  {
    char error_text[30]="";
    fits_get_errstatus(status, error_text);
    multilog(log, LOG_ERR, "create_fits(): Error creating fits file: %s. Error: %d -- %s\n", filename, status, error_text);
    return -1;
  }
    
  //
  // Add the core keywords
  //
  // SIMPLE
  int simple = TRUE;  
  char key_simple[FLEN_KEYWORD] = "SIMPLE";

  if ( fits_write_key(*fptr, TLOGICAL, key_simple, &simple, "conforms to FITS standard", &status) )
  {
    char error_text[30]="";
    fits_get_errstatus(status, error_text);
    multilog(log, LOG_ERR, "create_fits(): Error writing fits key: %s to file %s. Error: %d -- %s\n", key_simple, filename, status, error_text);
    return -1;
  }

  // BITPIX
  long bitpix = 8;
  char key_bitpix[FLEN_KEYWORD] = "BITPIX";

  if ( fits_write_key(*fptr, TLONG, key_bitpix, &bitpix, "array data type", &status) )
  {
    char error_text[30]="";
    fits_get_errstatus(status, error_text);
    multilog(log, LOG_ERR, "create_fits(): Error writing fits key: %s to file %s. Error: %d -- %s\n", key_bitpix, filename, status, error_text);
    return -1;
  }

  // NAXIS
  long naxis = 0;
  char key_naxis[FLEN_KEYWORD] = "NAXIS";

  if ( fits_write_key(*fptr, TLONG, key_naxis, &naxis, "number of array dimensions", &status) )
  {
    char error_text[30]="";
    fits_get_errstatus(status, error_text);
    multilog(log, LOG_ERR, "create_fits(): Error writing fits key: %s to file %s. Error: %d -- %s\n", key_naxis, filename, status, error_text);
    return -1;
  }
  
  // FITS citation comment 1
  if ( fits_write_comment(*fptr, "FITS (Flexible Image Transport System) format is defined in 'Astronomy", &status) )
  {
    char error_text[30]="";
    fits_get_errstatus(status, error_text);
    multilog(log, LOG_ERR, "create_fits(): Error writing fits comment to file %s. Error: %d -- %s\n", filename, status, error_text);
    return -1;
  }

  // FITS citation comment 2
  if ( fits_write_comment(*fptr, "and Astrophysics', volume 376, page 359; bibcode: 2001A&A...376..359H", &status) )
  {
    char error_text[30]="";
    fits_get_errstatus(status, error_text);
    multilog(log, LOG_ERR, "create_fits(): Error writing fits comment to file %s. Error: %d -- %s\n", filename, status, error_text);
    return -1;
  }

  // TIME
  char key_time[FLEN_KEYWORD] = "TIME";
  long unix_time = ctx->unix_time;
  if ( fits_write_key(*fptr, TLONG, key_time, &(unix_time), "Unix time (seconds)", &status) )
  {
    char error_text[30]="";
    fits_get_errstatus(status, error_text);
    multilog(log, LOG_ERR, "create_fits(): Error writing fits key: %s to file %s. Error: %d -- %s\n", key_time, filename, status, error_text);
    return -1;
  }

  // MILLITIM
  char key_millitim[FLEN_KEYWORD] = "MILLITIM";
  int unix_millitime = ctx->unix_time_msec;
  if ( fits_write_key(*fptr, TINT, key_millitim, &(unix_millitime), "Milliseconds since TIME", &status) )
  {
    char error_text[30]="";
    fits_get_errstatus(status, error_text);
    multilog(log, LOG_ERR, "create_fits(): Error writing fits key: %s to file %s. Error: %d -- %s\n", key_millitim, filename, status, error_text);
    return -1;
  }

  // INTTIME
  char key_inttime[FLEN_KEYWORD] = "INTTIME";
  float int_time_sec = (float)ctx->int_time_msec / 1000.0;
  if ( fits_write_key(*fptr, TFLOAT, key_inttime, &(int_time_sec), "Integration time (s)", &status) )
  {
    char error_text[30]="";
    fits_get_errstatus(status, error_text);
    multilog(log, LOG_ERR, "create_fits(): Error writing fits key: %s to file %s. Error: %d -- %s\n", key_inttime, filename, status, error_text);
    return -1;
  }
  
  // MARKER  
  char key_marker[FLEN_KEYWORD] = "MARKER";
  int marker = ctx->obs_marker_number;

  if (fits_write_key(*fptr, TINT, key_marker, &marker, "Data offset marker (all channels should match)", &status))
  {
    char error_text[30]="";
    fits_get_errstatus(status, error_text);
    multilog(log, LOG_ERR, "Error writing fits key %s into HDU. Error: %d -- %s\n", key_marker, status, error_text);
    return EXIT_FAILURE;
  }

  // PROJID
  char key_projid[FLEN_KEYWORD] = "PROJID";

  if ( fits_write_key(*fptr, TSTRING, key_projid, ctx->proj_id, "MWA Project Id", &status) )
  {
    char error_text[30]="";
    fits_get_errstatus(status, error_text);
    multilog(log, LOG_ERR, "create_fits(): Error writing fits key: %s to file %s. Error: %d -- %s\n", key_projid, filename, status, error_text);
    return -1;
  }

  // OBSID
  char key_obsid[FLEN_KEYWORD] = "OBSID";

  if ( fits_write_key(*fptr, TLONG, key_obsid, &(ctx->obs_id), "MWA Observation Id", &status) )
  {
    char error_text[30]="";
    fits_get_errstatus(status, error_text);
    multilog(log, LOG_ERR, "create_fits(): Error writing fits key: %s to file %s. Error: %d -- %s\n", key_obsid, filename, status, error_text);
    return -1;
  }

  return(EXIT_SUCCESS);
}

/**
 * 
 *  @brief Closes the fits file.
 *  @param[in] client A pointer to the dada_client_t object.
 *  @param[in,out] fptr Pointer to a pointer to the fitsfile structure.
 *  @returns EXIT_SUCCESS on success, or EXIT_FAILURE if there was an error.
 */
int close_fits(dada_client_t *client, fitsfile **fptr)
{
  assert(client != 0);
  dada_db_s* ctx = (dada_db_s*) client->context;

  assert(ctx->log != 0);
  multilog_t *log = (multilog_t *) ctx->log;  

  multilog(log, LOG_DEBUG, "close_fits() called.\n");
  
  int status = 0;

  if (*fptr != NULL)
  {    
    if (fits_close_file(*fptr, &status))
    {
      char error_text[30]="";
      fits_get_errstatus(status, error_text);
      multilog(log, LOG_ERR, "Error closing fits file. Error: %d -- %s\n", status, error_text);
      return EXIT_FAILURE;
    }
    else
    {
      *fptr = NULL;
    }
  }
  else
  {
    multilog(log, LOG_WARNING, "Fits file is already closed.\n");
  }

  return(EXIT_SUCCESS);
}

/**
 * 
 *  @brief Opens a fits file for reading.
 *  @param[in] client A pointer to the dada_client_t object.
 *  @param[in,out] fptr Pointer to a pointer of the openned fits file.
 *  @param[in] filename Full path/name of the file to be openned.
 *  @returns EXIT_SUCCESS on success, or EXIT_FAILURE if there was an error.
 */
int open_fits(dada_client_t *client, fitsfile **fptr, const char *filename)
{
  assert(client != 0);
  dada_db_s* ctx = (dada_db_s*) client->context;

  assert(ctx->log != 0);
  multilog_t *log = (multilog_t *) ctx->log;  
  
  int status = 0;

  if (fits_open_file(fptr, filename, READONLY, &status))
  {
    char error_text[30]="";
    fits_get_errstatus(status, error_text);
    multilog(log, LOG_ERR, "Error openning fits file %s. Error: %d -- %s\n", filename, status, error_text);
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

/**
 * 
 *  @brief Creates a new IMGHDU in an existing fits file.
 *  @param[in] client A pointer to the dada_client_t object. 
 *  @param[in] fptr Pointer to the fits file we will write to.
 *  @param[in] unix_time The Unix time for this integration / timestep.
 *  @param[in] unix_millisecond_time Number of milliseconds since the last integer of unix_time.
 *  @param[in] marker The artificial counter we use to keep track of which integration/timestep this is within the observation (0 based).
 *  @param[in] baselines The number of baselines in the data (used to calculate number of elements).
 *  @param[in] fine_channels The number of fine channels (used to calculate number of elements).
 *  @param[in] polarisations The number of pols in each antenna-normally 2 (used to calculate number of elements).
 *  @param[in] int_time The integration time of the observation (milliseconds).
 *  @param[in] buffer The pointer to the data to write into the HDU.
 *  @param[in] bytes The number of bytes in the buffer to write.
 *  @returns EXIT_SUCCESS on success, or EXIT_FAILURE if there was an error.
 */
int create_fits_imghdu(dada_client_t *client, fitsfile *fptr, time_t unix_time, int unix_millisecond_time, int marker, int baselines, 
                       int fine_channels, int polarisations, float int_time_msec, float *buffer, uint64_t bytes)
{
  //
  // Each imagehdu will be [baseline][freq][pols][real][imaginary] for an integration
  // So, if we think of the HDU as a 2d matrix, it would be:
  //
  // NAXIS1 = NINPUTS_XGPU * (NINPUTS_XGPU+2) / 8 == (TILES * TILES + 1)/ 2 == BASELINES
  // NAXIS2 = FINE_CHAN * NPOL * NPOL * 2 (real/imag)
  //
  // [time][baseline][pol][freq]
  //
  //           Freq/Pol  
  // Baseline  xx_Ch01 xx_Ch02 xx_ChNN yy_Ch01 yy_Ch02 yy_ChNN xy_Ch01 Ch02_yx...
  //    1-1    r,i     r,i     r,i     r,i     r,i     r,i     r,i     r,i    ...
  //    1-2    r,i     r,i     r,i     r,i     r,i     r,i     r,i     r,i
  //    1-3
  //    ...
  //
  //
  //  Flattened it looks like this:
  //  =============================
  //  [1-1|xx|Ch01|r],[1-1|xx|Ch01|i],[1-1|xx|Ch02|r],[1-1|xx|Ch02|i],...[1-1|xx|ChNN|r],[1-1|xx|ChNN|i]...
  //  [1-1|yy|Ch01|r],[1-1|yy|Ch01|i],[1-1|yy|Ch02|r],[1-1|yy|Ch02|i],...[1-1|yy|ChNN|r],[1-1|yy|ChNN|i]...
  //  [1-1|xy|Ch01|r],[1-1|xy|Ch01|i],[1-1|xy|Ch02|r],[1-1|xy|Ch02|i],...[1-1|xy|ChNN|r],[1-1|xy|ChNN|i]...
  //  [1-1|yx|Ch01|r],[1-1|yx|Ch01|i],[1-1|yx|Ch02|r],[1-1|yx|Ch02|i],...[1-1|yx|ChNN|r],[1-1|yx|ChNN|i]
  //
  //  [1-2|xx|Ch01|r],[1-2|xx|Ch01|i],[1-2|xx|Ch02|r],[1-2|xx|Ch02|i],...[1-2|xx|ChNN|r],[1-2|xx|ChNN|i]...
  //  [1-2|yy|Ch01|r],[1-2|yy|Ch01|i],[1-2|yy|Ch02|r],[1-2|yy|Ch02|i],...[1-2|yy|ChNN|r],[1-2|yy|ChNN|i]...
  //  [1-2|xy|Ch01|r],[1-2|xy|Ch01|i],[1-2|xy|Ch02|r],[1-2|xy|Ch02|i],...[1-2|xy|ChNN|r],[1-2|xy|ChNN|i]...
  //  [1-2|yx|Ch01|r],[1-2|yx|Ch01|i],[1-2|yx|Ch02|r],[1-2|yx|Ch02|i],...[1-2|yx|ChNN|r],[1-2|yx|ChNN|i]...
  //  
  //
  //  TileA TileB  Pol  Freq  data
  //  1     1       xx  ch01  r,i
  //  1     1       xx  ch02  r,i
  //  1     1       xx  ...
  //  1     1       xx  chNN  r,i
  //  1     1       yy  ch01  r,i
  //  1     1       yy  ch02  r,i
  //  1     1       yy  ...
  //  1     1       yy  chNN  r,i
  //  1     1       xy  ch01  r,i
  //  1     1       xy  ch02  r,i
  //  1     1       xy  ...
  //  1     1       xy  chNN  r,i
  //  1     1       yx  ch01  r,i
  //  1     1       yx  ch02  r,i
  //  1     1       yx  ...
  //  1     1       yx  chNN  r,i
  //  1     2
  //  1    ...
  //  1    128
  //  2     2
  //  2    ...
  //  2    128
  //  ...
  //
  assert(client != 0);
  dada_db_s* ctx = (dada_db_s*) client->context;

  assert(ctx->log != 0);
  multilog_t *log = (multilog_t *) ctx->log;  
  
  int status = 0;
  int bitpix = FLOAT_IMG;  //complex(r,i)  = 2x4 bytes  == 8 bytes
  long naxis = 2; 
  uint64_t axis1_rows = baselines;
  uint64_t axis2_cols = fine_channels * polarisations * polarisations * 2;   //  we x2 as we store real and imaginary;
  long naxes[2] = { axis1_rows, axis2_cols };  

  multilog(log, LOG_DEBUG, "Creating new HDU in fits file with dimensions %lld x %lld...\n", (long long)axis1_rows, (long long)axis2_cols);

  // Create new IMGHDU    
  if (fits_create_img(fptr, bitpix, naxis, naxes, &status))
  {
    char error_text[30]="";
    fits_get_errstatus(status, error_text);
    multilog(log, LOG_ERR, "Error creating HDU in fits file. Error: %d -- %s\n", status, error_text);
    return EXIT_FAILURE;
  }  

  // TIME  - cotter uses this to align each channel
  char key_time[FLEN_KEYWORD] = "TIME";

  if (fits_write_key(fptr, TLONG, key_time, &unix_time, (char *) "Unix time (seconds)", &status))
  {
    char error_text[30]="";
    fits_get_errstatus(status, error_text);
    multilog(log, LOG_ERR, "Error writing key %s into HDU. Error: %d -- %s\n", key_time, status, error_text);
    return EXIT_FAILURE;
  }
  
  // MILLITIME - provides millisecond component of TIME
  char key_millitim[FLEN_KEYWORD] = "MILLITIM";

  if (fits_update_key(fptr, TINT, key_millitim, &unix_millisecond_time, (char *)"Milliseconds since TIME",&status))
  {
    char error_text[30]="";
    fits_get_errstatus(status, error_text);
    multilog(log, LOG_ERR, "Error writing key %s into HDU. Error: %d -- %s\n", key_millitim, status, error_text);
    return EXIT_FAILURE;
  }

  // INTTIME
  char key_inttime[FLEN_KEYWORD] = "INTTIME";
  float int_time_sec = (float)int_time_msec / 1000.0;
  if (fits_write_key(fptr, TFLOAT, key_inttime, &int_time_sec, (char*)"Integration time (s)", &status))
  {
    char error_text[30]="";
    fits_get_errstatus(status, error_text);
    multilog(log, LOG_ERR, "Error writing fits key %s into HDU. Error: %d -- %s\n", key_inttime, status, error_text);
    return EXIT_FAILURE;
  }

  // MARKER  
  char key_marker[FLEN_KEYWORD] = "MARKER";

  if (fits_write_key(fptr, TINT, key_marker, &marker, (char*)"Data offset marker (all channels should match)", &status))
  {
    char error_text[30]="";
    fits_get_errstatus(status, error_text);
    multilog(log, LOG_ERR, "Error writing fits key %s into HDU. Error: %d -- %s\n", key_marker, status, error_text);
    return EXIT_FAILURE;
  }

  /* Write the array */
  long nelements = bytes / (abs(bitpix) / 8);

  // Check that number of elements * bytes per element matches what we expect  
  u_int64_t expected_bytes = (axis1_rows * axis2_cols * (abs(bitpix) / 8));
  if (bytes != expected_bytes)
  {
    multilog(log, LOG_ERR, "HDU bytes (%lu bytes) does not match calculated size from header parameters (%lu bytes).\n", bytes, expected_bytes);
    return EXIT_FAILURE;
  }

  // Actually write the HDU data
  if (fits_write_img(fptr, TFLOAT, 1, nelements, buffer, &status))
  {
    char error_text[30]="";
    fits_get_errstatus(status, error_text);
    multilog(log, LOG_ERR, "Error writing data into HDU in fits file. Error: %d -- %s\n", status, error_text);
    return EXIT_FAILURE;
  }
      
  return EXIT_SUCCESS;
}

/*int read_metafits(fitsfile *fptr_metafits, metafits_info *mptr)
{  
  int status = 0;
  
  // INTTIME
  float inttime = 0;  
  char key_inittime[FLEN_KEYWORD] = "INTTIME";

  multilog(log, LOG_INFO, "Reading %s from metafits\n", key_inittime);  
  if ( fits_read_key(fptr_metafits, TFLOAT, key_inittime, &inttime, NULL, &status) )
  {    
    char error_text[30]="";
    fits_get_errstatus(status, error_text);
    multilog(log, LOG_ERR, "Error reading metafits key: %s in file %s. Error: %d -- %s\n", key_inittime, fptr_metafits->Fptr->filename, status, error_text);
    return EXIT_FAILURE;
  }
  mptr->inttime = inttime;

  // PROJECT
  char project[FLEN_VALUE];  
  char key_project[FLEN_KEYWORD] = "PROJECT";

  multilog(log, LOG_INFO, "Reading PROJECT from metafits\n");
  if ( fits_read_key(fptr_metafits, TSTRING, key_project, project, NULL, &status) )
  {
    char error_text[30]="";
    fits_get_errstatus(status, error_text);
    multilog(log, LOG_ERR, "Error reading metafits key: %s in file %s. Error: %d -- %s\n", key_project, fptr_metafits->Fptr->filename, status, error_text);
    return EXIT_FAILURE;
  }
  mptr->project = strdup(project);

  // GPSTIME / OBSID
  long obsid = 0;  
  char key_gpstime[FLEN_KEYWORD] = "GPSTIME";

  multilog(log, LOG_INFO, "Reading GPSTIME from metafits\n");
  if ( fits_read_key(fptr_metafits, TLONG, key_gpstime, &obsid, NULL, &status) )
  {
    char error_text[30]="";
    fits_get_errstatus(status, error_text);
    multilog(log, LOG_ERR, "Error reading metafits key: %s in file %s. Error: %d -- %s\n", key_gpstime, fptr_metafits->Fptr->filename, status, error_text);
    return EXIT_FAILURE;
  }
  mptr->obsid = obsid;

  return (EXIT_SUCCESS);
}*/