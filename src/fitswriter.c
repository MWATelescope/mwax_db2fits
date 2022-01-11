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

#include "fitswriter.h"
#include "fitswriter.h"
#include "global.h"
#include "multilog.h"
#include "utils.h"
#include "version.h"

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
  dada_db_s *ctx = (dada_db_s *)client->context;

  assert(ctx->log != 0);
  multilog_t *log = (multilog_t *)client->log;

  int status = 0;

  multilog(log, LOG_INFO, "create_fits(): Creating new fits file %s...\n", filename);

  // So CFITSIO overwrites the file, we should prefix the filename with !
  int len = strlen(filename) + 2;
  char cfitsio_filename[len];
  sprintf(cfitsio_filename, "!%s", filename);

  // Create a new blank fits file
  if (fits_create_file(fptr, cfitsio_filename, &status))
  {
    char error_text[30] = "";
    fits_get_errstatus(status, error_text);
    multilog(log, LOG_ERR, "create_fits(): Error creating fits file: %s. Error: %d -- %s\n", filename, status, error_text);
    return -1;
  }

  //
  // Add the core keywords
  //
  // SIMPLE
  int simple = MWA_FITS_VALUE_SIMPLE;

  if (fits_write_key(*fptr, TLOGICAL, MWA_FITS_KEY_SIMPLE, &(simple), "conforms to FITS standard", &status))
  {
    char error_text[30] = "";
    fits_get_errstatus(status, error_text);
    multilog(log, LOG_ERR, "create_fits(): Error writing fits key: %s to file %s. Error: %d -- %s\n", MWA_FITS_KEY_SIMPLE, filename, status, error_text);
    return -1;
  }

  // BITPIX
  long bitpix = MWA_FITS_VALUE_BITPIX;

  if (fits_write_key(*fptr, TLONG, MWA_FITS_KEY_BITPIX, &(bitpix), "array data type", &status))
  {
    char error_text[30] = "";
    fits_get_errstatus(status, error_text);
    multilog(log, LOG_ERR, "create_fits(): Error writing fits key: %s to file %s. Error: %d -- %s\n", MWA_FITS_KEY_BITPIX, filename, status, error_text);
    return -1;
  }

  // NAXIS
  long naxis = MWA_FITS_VALUE_NAXIS;

  if (fits_write_key(*fptr, TLONG, MWA_FITS_KEY_NAXIS, &(naxis), "number of array dimensions", &status))
  {
    char error_text[30] = "";
    fits_get_errstatus(status, error_text);
    multilog(log, LOG_ERR, "create_fits(): Error writing fits key: %s to file %s. Error: %d -- %s\n", MWA_FITS_KEY_NAXIS, filename, status, error_text);
    return -1;
  }

  // FITS citation comment 1
  if (fits_write_comment(*fptr, "FITS (Flexible Image Transport System) format is defined in 'Astronomy", &status))
  {
    char error_text[30] = "";
    fits_get_errstatus(status, error_text);
    multilog(log, LOG_ERR, "create_fits(): Error writing fits comment to file %s. Error: %d -- %s\n", filename, status, error_text);
    return -1;
  }

  // FITS citation comment 2
  if (fits_write_comment(*fptr, "and Astrophysics', volume 376, page 359; bibcode: 2001A&A...376..359H", &status))
  {
    char error_text[30] = "";
    fits_get_errstatus(status, error_text);
    multilog(log, LOG_ERR, "create_fits(): Error writing fits comment to file %s. Error: %d -- %s\n", filename, status, error_text);
    return -1;
  }

  // CORR_VERS
  int corr_ver = MWA_FITS_VALUE_CORR_VER;

  if (fits_write_key(*fptr, TINT, MWA_FITS_KEY_CORR_VER, &(corr_ver), "MWA Correlator Version", &status))
  {
    char error_text[30] = "";
    fits_get_errstatus(status, error_text);
    multilog(log, LOG_ERR, "create_fits(): Error writing fits key: %s to file %s. Error: %d -- %s\n", MWA_FITS_KEY_CORR_VER, filename, status, error_text);
    return -1;
  }

  // mwax_u2s version
  if (fits_write_key(*fptr, TSTRING, MWA_FITS_KEY_MWAX_U2S_VERSION, &ctx->mwax_u2s_version, "MWAX u2s version", &status))
  {
    char error_text[30] = "";
    fits_get_errstatus(status, error_text);
    multilog(log, LOG_ERR, "create_fits(): Error writing fits key: %s to file %s. Error: %d -- %s\n", MWA_FITS_KEY_MWAX_U2S_VERSION, filename, status, error_text);
    return -1;
  }

  // mwax_db2correlate2db_version
  if (fits_write_key(*fptr, TSTRING, MWA_FITS_KEY_MWAX_DB2CORRELATE2DB_VERSION, &ctx->mwax_db2correlate2db_version, "MWAX db2correlate2db version", &status))
  {
    char error_text[30] = "";
    fits_get_errstatus(status, error_text);
    multilog(log, LOG_ERR, "create_fits(): Error writing fits key: %s to file %s. Error: %d -- %s\n", MWA_FITS_KEY_MWAX_DB2CORRELATE2DB_VERSION, filename, status, error_text);
    return -1;
  }

  // mwax_db2fits_version
  char mwax_db2fits_version[MWAX_VERSION_STRING_LEN];
  snprintf(mwax_db2fits_version, MWAX_VERSION_STRING_LEN, "%d.%d.%d", MWAX_DB2FITS_VERSION_MAJOR, MWAX_DB2FITS_VERSION_MINOR, MWAX_DB2FITS_VERSION_PATCH);

  if (fits_write_key(*fptr, TSTRING, MWA_FITS_KEY_MWAX_DB2FITS_VERSION, &(mwax_db2fits_version), "MWAX db2fits version", &status))
  {
    char error_text[30] = "";
    fits_get_errstatus(status, error_text);
    multilog(log, LOG_ERR, "create_fits(): Error writing fits key: %s to file %s. Error: %d -- %s\n", MWA_FITS_KEY_MWAX_DB2FITS_VERSION, filename, status, error_text);
    return -1;
  }

  // Data format comment1
  if (fits_write_comment(*fptr, "Visibilities: 1 integration per HDU: [baseline][finechan][pol][r,i]", &status))
  {
    char error_text[30] = "";
    fits_get_errstatus(status, error_text);
    multilog(log, LOG_ERR, "create_fits(): Error writing visibility format comment to file %s. Error: %d -- %s\n", filename, status, error_text);
    return -1;
  }

  // Data format comment2
  if (fits_write_comment(*fptr, "Weights: 1 integration per HDU: [baseline][pol][weight]", &status))
  {
    char error_text[30] = "";
    fits_get_errstatus(status, error_text);
    multilog(log, LOG_ERR, "create_fits(): Error writing visibility format comment to file %s. Error: %d -- %s\n", filename, status, error_text);
    return -1;
  }

  // MARKER
  int marker = ctx->obs_marker_number;

  if (fits_write_key(*fptr, TINT, MWA_FITS_KEY_MARKER, &marker, "Data offset marker (all channels should match)", &status))
  {
    char error_text[30] = "";
    fits_get_errstatus(status, error_text);
    multilog(log, LOG_ERR, "Error writing fits key %s into HDU. Error: %d -- %s\n", MWA_FITS_KEY_MARKER, status, error_text);
    return EXIT_FAILURE;
  }

  // TIME
  long unix_time = ctx->unix_time;

  if (fits_write_key(*fptr, TLONG, MWA_FITS_KEY_TIME, &(unix_time), "Unix time (seconds)", &status))
  {
    char error_text[30] = "";
    fits_get_errstatus(status, error_text);
    multilog(log, LOG_ERR, "create_fits(): Error writing fits key: %s to file %s. Error: %d -- %s\n", MWA_FITS_KEY_TIME, filename, status, error_text);
    return -1;
  }

  // MILLITIM
  int unix_millitime = ctx->unix_time_msec;

  if (fits_write_key(*fptr, TINT, MWA_FITS_KEY_MILLITIM, &(unix_millitime), "Milliseconds since TIME", &status))
  {
    char error_text[30] = "";
    fits_get_errstatus(status, error_text);
    multilog(log, LOG_ERR, "create_fits(): Error writing fits key: %s to file %s. Error: %d -- %s\n", MWA_FITS_KEY_MILLITIM, filename, status, error_text);
    return -1;
  }

  // PROJID
  if (fits_write_key(*fptr, TSTRING, MWA_FITS_KEY_PROJID, ctx->proj_id, "MWA Project Id", &status))
  {
    char error_text[30] = "";
    fits_get_errstatus(status, error_text);
    multilog(log, LOG_ERR, "create_fits(): Error writing fits key: %s to file %s. Error: %d -- %s\n", MWA_FITS_KEY_PROJID, filename, status, error_text);
    return -1;
  }

  // OBSID
  if (fits_write_key(*fptr, TLONG, MWA_FITS_KEY_OBSID, &(ctx->obs_id), "MWA Observation Id", &status))
  {
    char error_text[30] = "";
    fits_get_errstatus(status, error_text);
    multilog(log, LOG_ERR, "create_fits(): Error writing fits key: %s to file %s. Error: %d -- %s\n", MWA_FITS_KEY_OBSID, filename, status, error_text);
    return -1;
  }

  // FINECHAN
  float finechan = ctx->fine_chan_width_hz / 1000.0f;

  if (fits_write_key(*fptr, TFLOAT, MWA_FITS_KEY_FINECHAN, &(finechan), "[kHz] Fine channel width", &status))
  {
    char error_text[30] = "";
    fits_get_errstatus(status, error_text);
    multilog(log, LOG_ERR, "create_fits(): Error writing fits key: %s to file %s. Error: %d -- %s\n", MWA_FITS_KEY_FINECHAN, filename, status, error_text);
    return -1;
  }

  // NFINECHS
  int nchans = ctx->nfine_chan;

  if (fits_write_key(*fptr, TINT, MWA_FITS_KEY_NFINECHS, &(nchans), "Number of fine channels in this coarse channel", &status))
  {
    char error_text[30] = "";
    fits_get_errstatus(status, error_text);
    multilog(log, LOG_ERR, "create_fits(): Error writing fits key: %s to file %s. Error: %d -- %s\n", MWA_FITS_KEY_NFINECHS, filename, status, error_text);
    return -1;
  }

  // INTTIME
  float int_time_sec = (float)ctx->int_time_msec / 1000.0;

  if (fits_write_key(*fptr, TFLOAT, MWA_FITS_KEY_INTTIME, &(int_time_sec), "Integration time (s)", &status))
  {
    char error_text[30] = "";
    fits_get_errstatus(status, error_text);
    multilog(log, LOG_ERR, "create_fits(): Error writing fits key: %s to file %s. Error: %d -- %s\n", MWA_FITS_KEY_INTTIME, filename, status, error_text);
    return -1;
  }

  // NINPUTS
  int ninputs = ctx->ninputs;

  if (fits_write_key(*fptr, TINT, MWA_FITS_KEY_NINPUTS, &(ninputs), "Number of rf inputs into the correlation products", &status))
  {
    char error_text[30] = "";
    fits_get_errstatus(status, error_text);
    multilog(log, LOG_ERR, "create_fits(): Error writing fits key: %s to file %s. Error: %d -- %s\n", MWA_FITS_KEY_NINPUTS, filename, status, error_text);
    return -1;
  }

  // CORR_HOST
  if (fits_write_key(*fptr, TSTRING, MWA_FITS_KEY_CORR_HOST, ctx->hostname, "Correlator host", &status))
  {
    char error_text[30] = "";
    fits_get_errstatus(status, error_text);
    multilog(log, LOG_ERR, "create_fits(): Error writing fits key: %s to file %s. Error: %d -- %s\n", MWA_FITS_KEY_CORR_HOST, filename, status, error_text);
    return -1;
  }

  // CORR_CHAN
  // This is 0 based, whereas the PSRDADA value is 1 based.
  int corr_chan = ctx->corr_coarse_channel - 1;

  if (fits_write_key(*fptr, TINT, MWA_FITS_KEY_CORR_CHAN, &(corr_chan), "Correlator coarse channel (0 to N-1)", &status))
  {
    char error_text[30] = "";
    fits_get_errstatus(status, error_text);
    multilog(log, LOG_ERR, "create_fits(): Error writing fits key: %s to file %s. Error: %d -- %s\n", MWA_FITS_KEY_CORR_CHAN, filename, status, error_text);
    return -1;
  }

  // MC_IP
  if (fits_write_key(*fptr, TSTRING, MWA_FITS_KEY_MC_IP, ctx->multicast_ip, "Multicast IP", &status))
  {
    char error_text[30] = "";
    fits_get_errstatus(status, error_text);
    multilog(log, LOG_ERR, "create_fits(): Error writing fits key: %s to file %s. Error: %d -- %s\n", MWA_FITS_KEY_MC_IP, filename, status, error_text);
    return -1;
  }

  // MC_PORT
  int multicast_port = 0;

  if (fits_write_key(*fptr, TINT, MWA_FITS_KEY_MC_PORT, &(multicast_port), "Multicast Port", &status))
  {
    char error_text[30] = "";
    fits_get_errstatus(status, error_text);
    multilog(log, LOG_ERR, "create_fits(): Error writing fits key: %s to file %s. Error: %d -- %s\n", MWA_FITS_KEY_MC_PORT, filename, status, error_text);
    return -1;
  }

  return (EXIT_SUCCESS);
}

/**
 *
 *  @brief Closes the fits file, and renames it to remove the .tmp extension.
 *  @param[in] client A pointer to the dada_client_t object.
 *  @param[in,out] fptr Pointer to a pointer to the fitsfile structure.
 *  @returns EXIT_SUCCESS on success, or EXIT_FAILURE if there was an error.
 */
int close_fits(dada_client_t *client, fitsfile **fptr)
{
  assert(client != 0);
  dada_db_s *ctx = (dada_db_s *)client->context;

  assert(ctx->log != 0);
  multilog_t *log = (multilog_t *)ctx->log;

  multilog(log, LOG_DEBUG, "close_fits(): Starting.\n");

  int status = 0;

  if (*fptr != NULL)
  {
    if (fits_close_file(*fptr, &status))
    {
      char error_text[30] = "";
      fits_get_errstatus(status, error_text);
      multilog(log, LOG_ERR, "close_fits(): Error closing fits file. Error: %d -- %s\n", status, error_text);
      return EXIT_FAILURE;
    }
    else
    {
      *fptr = NULL;
    }
  }
  else
  {
    multilog(log, LOG_WARNING, "close_fits(): Fits file is already closed.\n");
  }

  // At this point the temp fits file is closed. We should now rename it to .fits so it is picked up for archiving
  if (rename(ctx->temp_fits_filename, ctx->fits_filename) == 0)
  {
    multilog(log, LOG_INFO, "close_fits(): rename of %s to %s successful.\n", ctx->temp_fits_filename, ctx->fits_filename);
  }
  else
  {
    multilog(log, LOG_ERR, "close_fits(): ERROR renaming %s to %s.\n", ctx->temp_fits_filename, ctx->fits_filename);
  }

  return (EXIT_SUCCESS);
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
  dada_db_s *ctx = (dada_db_s *)client->context;

  assert(ctx->log != 0);
  multilog_t *log = (multilog_t *)ctx->log;

  int status = 0;

  if (fits_open_file(fptr, filename, READONLY, &status))
  {
    char error_text[30] = "";
    fits_get_errstatus(status, error_text);
    multilog(log, LOG_ERR, "open_fits(): Error openning fits file %s. Error: %d -- %s\n", filename, status, error_text);
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

/**
 *
 *  @brief Creates a new visibility IMGHDU in an existing fits file.
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
int create_fits_visibilities_imghdu(dada_client_t *client, fitsfile *fptr, time_t unix_time, int unix_millisecond_time, int marker,
                                    int baselines, int fine_channels, int polarisations, float *buffer, uint64_t bytes)
{
  //
  // Each imagehdu will be [baseline][freq][pols][real][imaginary] for an integration
  // So, if we think of the HDU as a 2d matrix, it would be:
  //
  // NAXIS1 is rows, NAXIS2 is cols. We want NAXIS1 < NAXIS2 for efficiency
  // NAXIS1 = FINE_CHAN * NPOL * NPOL * 2 (real/imag)
  // NAXIS2 = NINPUTS * (NINPUTS+2) / 8 == (TILES * TILES + 1)/ 2 == BASELINES
  //
  //
  // [time][baseline][freq][pol]
  //
  //           Freq/Pol
  // Baseline  Ch01xx  Ch01xy  Ch01yx  Ch01yy  Ch02xx  Ch02xy  Ch02yx  Ch02yy ...
  //    1-1    r,i     r,i     r,i     r,i     r,i     r,i     r,i     r,i    ...
  //    1-2    r,i     r,i     r,i     r,i     r,i     r,i     r,i     r,i
  //    1-3
  //    ...
  //
  //
  //  Flattened it looks like this:
  //  =============================
  //  [1-1|Ch01|xx|r],[1-1|Ch01|xx|i],[1-1|Ch01|xy|r],[1-1|Ch01|xy|i],...[1-1|ChNN|yy|r],[1-1|ChNN|yy|i]...
  //  [1-1|Ch02|xx|r],[1-1|Ch02|xx|i],[1-1|Ch02|xy|r],[1-1|Ch02|xy|i],...[1-1|ChNN|yy|r],[1-1|ChNN|yy|i]...
  //  [1-1|Ch03|xx|r],[1-1|Ch03|xx|i],[1-1|Ch03|xy|r],[1-1|Ch03|xy|i],...[1-1|ChNN|yy|r],[1-1|ChNN|yy|i]...
  //  [1-1|Ch04|xx|r],[1-1|Ch04|xx|i],[1-1|Ch04|xy|r],[1-1|Ch04|xy|i],...[1-1|ChNN|yy|r],[1-1|ChNN|yy|i]...
  //
  //  [1-2|Ch01|xx|r],[1-2|Ch01|xx|i],[1-2|Ch01|xy|r],[1-2|Ch01|xy|i],...[1-2|ChNN|yy|r],[1-2|ChNN|yy|i]...
  //  [1-2|Ch02|xx|r],[1-2|Ch02|xx|i],[1-2|Ch02|xy|r],[1-2|Ch02|xy|i],...[1-2|ChNN|yy|r],[1-2|ChNN|yy|i]...
  //  [1-2|Ch03|xx|r],[1-2|Ch03|xx|i],[1-2|Ch03|xy|r],[1-2|Ch03|xy|i],...[1-2|ChNN|yy|r],[1-2|ChNN|yy|i]...
  //  [1-2|Ch04|xx|r],[1-2|Ch04|xx|i],[1-2|Ch04|xy|r],[1-2|Ch04|xy|i],...[1-2|ChNN|yy|r],[1-2|ChNN|yy|i]...
  //
  //  Tile order:
  //  1     1
  //  1     2
  //  1    ...
  //  1    128
  //  2     2
  //  2    ...
  //  2    128
  //  ...
  //
  assert(client != 0);
  dada_db_s *ctx = (dada_db_s *)client->context;

  assert(ctx->log != 0);
  multilog_t *log = (multilog_t *)ctx->log;

  int status = 0;
  int bitpix = FLOAT_IMG; //complex(r,i)  = 2x4 bytes
  long naxis = 2;
  uint64_t axis1_rows = fine_channels * polarisations * polarisations * 2; //  we x2 as we store real and imaginary;
  uint64_t axis2_cols = baselines;

  long naxes[2] = {axis1_rows, axis2_cols};

  multilog(log, LOG_DEBUG, "create_fits_visibilities_imghdu(): Creating new visibility HDU in fits file with dimensions %lld x %lld...\n", (long long)axis1_rows, (long long)axis2_cols);

  // Create new IMGHDU
  if (fits_create_img(fptr, bitpix, naxis, naxes, &status))
  {
    char error_text[30] = "";
    fits_get_errstatus(status, error_text);
    multilog(log, LOG_ERR, "create_fits_visibilities_imghdu(): Error creating visibility ImgHDU in fits file. Error: %d -- %s\n", status, error_text);
    return EXIT_FAILURE;
  }

  // TIME  - cotter uses this to align each channel
  char key_time[FLEN_KEYWORD] = "TIME";

  if (fits_write_key(fptr, TLONG, key_time, &unix_time, (char *)"Unix time (seconds)", &status))
  {
    char error_text[30] = "";
    fits_get_errstatus(status, error_text);
    multilog(log, LOG_ERR, "create_fits_visibilities_imghdu(): Error writing key %s into visibility HDU. Error: %d -- %s\n", key_time, status, error_text);
    return EXIT_FAILURE;
  }

  // MILLITIME - provides millisecond component of TIoffset_ME
  char key_millitim[FLEN_KEYWORD] = "MILLITIM";

  if (fits_update_key(fptr, TINT, key_millitim, &unix_millisecond_time, (char *)"Milliseconds since TIME", &status))
  {
    char error_text[30] = "";
    fits_get_errstatus(status, error_text);
    multilog(log, LOG_ERR, "create_fits_visibilities_imghdu(): Error writing %s into visibility HDU. Error: %d -- %s\n", key_millitim, status, error_text);
    return EXIT_FAILURE;
  }

  // MARKER
  char key_marker[FLEN_KEYWORD] = "MARKER";

  if (fits_write_key(fptr, TINT, key_marker, &marker, (char *)"Data offset marker (all channels should match)", &status))
  {
    char error_text[30] = "";
    fits_get_errstatus(status, error_text);
    multilog(log, LOG_ERR, "create_fits_visibilities_imghdu(): Error writing key %s into visibility HDU. Error: %d -- %s\n", key_marker, status, error_text);
    return EXIT_FAILURE;
  }

  /* Write the array */
  long nelements = bytes / (abs(bitpix) / 8);

  // Check that number of elements * bytes per element matches what we expect
  u_int64_t expected_bytes = (axis1_rows * axis2_cols * (abs(bitpix) / 8));
  if (bytes != expected_bytes)
  {
    multilog(log, LOG_ERR, "create_fits_visibilities_imghdu(): Visibility HDU bytes (%lu bytes) does not match calculated size from header parameters (%lu bytes).\n", bytes, expected_bytes);
    return EXIT_FAILURE;
  }

  // Actually write the HDU data
  if (fits_write_img(fptr, TFLOAT, 1, nelements, buffer, &status))
  {
    char error_text[30] = "";
    fits_get_errstatus(status, error_text);
    multilog(log, LOG_ERR, "create_fits_visibilities_imghdu(): Error writing data into visibility HDU in fits file. Error: %d -- %s\n", status, error_text);
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

/**
 *
 *  @brief Creates a new weights IMGHDU in an existing fits file.
 *  @param[in] client A pointer to the dada_client_t object.
 *  @param[in] fptr Pointer to the fits file we will write to.
 *  @param[in] unix_time The Unix time for this integration / timestep.
 *  @param[in] unix_millisecond_time Number of milliseconds since the last integer of unix_time.
 *  @param[in] marker The artificial counter we use to keep track of which integration/timestep this is within the observation (0 based).
 *  @param[in] baselines The number of baselines in the data (used to calculate number of elements).
 *  @param[in] polarisations The number of pols in each antenna-normally 2 (used to calculate number of elements).
 *  @param[in] int_time The integration time of the observation (milliseconds).
 *  @param[in] buffer The pointer to the data to write into the HDU.
 *  @param[in] bytes The number of bytes in the buffer to write.
 *  @returns EXIT_SUCCESS on success, or EXIT_FAILURE if there was an error.
 */
int create_fits_weights_imghdu(dada_client_t *client, fitsfile *fptr, time_t unix_time, int unix_millisecond_time, int marker,
                               int baselines, int polarisations, float *buffer, uint64_t bytes)
{
  // NAXIS1 is rows, NAXIS2 is cols. We want NAXIS1 < NAXIS2 for efficiency
  // NAXIS1 = NPOL * NPOL
  // NAXIS2 = NINPUTS * (NINPUTS+2) / 8 == (TILES * TILES + 1)/ 2 == BASELINES
  //
  //           Weights
  // Baseline  xx      xy      yx      yy
  //    1-1    w       w       w       w
  //    1-2    w       w       w       w
  //    1-3    w       w       w       w
  //    ...
  //
  assert(client != 0);
  dada_db_s *ctx = (dada_db_s *)client->context;

  assert(ctx->log != 0);
  multilog_t *log = (multilog_t *)ctx->log;

  int status = 0;
  int bitpix = FLOAT_IMG; //complex(r,i)  = 2x4 bytes
  long naxis = 2;
  uint64_t axis1_rows = polarisations * polarisations;
  uint64_t axis2_cols = baselines;

  long naxes[2] = {axis1_rows, axis2_cols};

  multilog(log, LOG_DEBUG, "create_fits_weights_imghdu(): Creating new weights HDU in fits file with dimensions %lld x %lld...\n", (long long)axis1_rows, (long long)axis2_cols);

  // Create new IMGHDU
  if (fits_create_img(fptr, bitpix, naxis, naxes, &status))
  {
    char error_text[30] = "";
    fits_get_errstatus(status, error_text);
    multilog(log, LOG_ERR, "create_fits_weights_imghdu(): Error creating weights ImgHDU in fits file. Error: %d -- %s\n", status, error_text);
    return EXIT_FAILURE;
  }

  // TIME  - cotter uses this to align each channel
  char key_time[FLEN_KEYWORD] = "TIME";

  if (fits_write_key(fptr, TLONG, key_time, &unix_time, (char *)"Unix time (seconds)", &status))
  {
    char error_text[30] = "";
    fits_get_errstatus(status, error_text);
    multilog(log, LOG_ERR, "create_fits_weights_imghdu(): Error writing key %s into weights HDU. Error: %d -- %s\n", key_time, status, error_text);
    return EXIT_FAILURE;
  }

  // MILLITIME - provides millisecond component of TIME
  char key_millitim[FLEN_KEYWORD] = "MILLITIM";

  if (fits_update_key(fptr, TINT, key_millitim, &unix_millisecond_time, (char *)"Milliseconds since TIME", &status))
  {
    char error_text[30] = "";
    fits_get_errstatus(status, error_text);
    multilog(log, LOG_ERR, "create_fits_weights_imghdu(): Error writing key %s into weights HDU. Error: %d -- %s\n", key_millitim, status, error_text);
    return EXIT_FAILURE;
  }

  // MARKER
  char key_marker[FLEN_KEYWORD] = "MARKER";

  if (fits_write_key(fptr, TINT, key_marker, &marker, (char *)"Data offset marker (all channels should match)", &status))
  {
    char error_text[30] = "";
    fits_get_errstatus(status, error_text);
    multilog(log, LOG_ERR, "create_fits_weights_imghdu(): Error writing key %s into weights HDU. Error: %d -- %s\n", key_marker, status, error_text);
    return EXIT_FAILURE;
  }

  /* Write the array */
  long nelements = bytes / (abs(bitpix) / 8);

  // Check that number of elements * bytes per element matches what we expect
  u_int64_t expected_bytes = (axis1_rows * axis2_cols * (abs(bitpix) / 8));
  if (bytes != expected_bytes)
  {
    multilog(log, LOG_ERR, "create_fits_weights_imghdu(): Weights HDU bytes (%lu bytes) does not match calculated size from header parameters (%lu bytes).\n", bytes, expected_bytes);
    return EXIT_FAILURE;
  }

  // Actually write the HDU data
  if (fits_write_img(fptr, TFLOAT, 1, nelements, buffer, &status))
  {
    char error_text[30] = "";
    fits_get_errstatus(status, error_text);
    multilog(log, LOG_ERR, "create_fits_weights_imghdu(): Error writing data into weights HDU in fits file. Error: %d -- %s\n", status, error_text);
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
