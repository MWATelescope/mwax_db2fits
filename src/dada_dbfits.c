/**
 * @file dada_dbfits.c
 * @author Greg Sleap
 * @date 23 May 2018
 * @brief This is the code that drives the ring buffers
 *
 */
#include <stdio.h>
#include <errno.h>
#include "dada_dbfits.h"
#include "../mwax_common/mwax_global_defs.h" // From mwax-common
#include "global.h"
#include "utils.h"

/**
 * 
 *  @brief This is called at the begininning of each new 8 second sub-observation.
 *         We need check if we are in a new fits file or continuing the existing one.
 *  @param[in] client A pointer to the dada_client_t object.
 *  @returns EXIT_SUCCESS on success, or -1 if there was an error. 
 */
int dada_dbfits_open(dada_client_t *client)
{
  assert(client != 0);
  dada_db_s *ctx = (dada_db_s *)client->context;

  assert(ctx->log != 0);
  multilog_t *log = (multilog_t *)client->log;
  multilog(log, LOG_INFO, "dada_dbfits_open(): extracting params from dada header\n");

  // These need to be set for psrdada
  client->transfer_bytes = 0;
  client->optimal_bytes = 0;

  // we do not want to explicitly transfer the DADA header
  client->header_transfer = 0;

  // Read the command first
  strncpy(ctx->mode, "", MWAX_MODE_LEN);
  if (ascii_header_get(client->header, HEADER_MODE, "%s", &ctx->mode) == -1)
  {
    multilog(log, LOG_ERR, "dada_dbfits_open(): %s not found in header.\n", HEADER_MODE);
    return -1;
  }

  // Verify command is ok
  if (strlen(ctx->mode) > 0)
  {
    multilog(log, LOG_INFO, "dada_dbfits_open(): %s == %s\n", HEADER_MODE, ctx->mode);

    if (is_mwax_mode_correlator(ctx->mode) == 1)
    {
      // Normal operations
    }
    else if (is_mwax_mode_vcs(ctx->mode) == 1)
    {
      // Voltage_Start - don't correlate
      return EXIT_SUCCESS;
    }
    else if (is_mwax_mode_no_capture(ctx->mode) == 1)
    {
      // don't correlate
      return EXIT_SUCCESS;
    }
    else if (is_mwax_mode_quit(ctx->mode) == 1)
    {
      // We'll flag we want to quit
      set_quit(1);
      return EXIT_SUCCESS;
    }
    else
    {
      // Invalid command
      multilog(log, LOG_ERR, "dada_dbfits_open(): Error: %s '%s' not recognised.\n", HEADER_MODE, ctx->mode);
      return -1;
    }
  }
  else
  {
    // No command provided at all!
    multilog(log, LOG_ERR, "dada_dbfits_open(): Error: an empty %s was provided.\n", HEADER_MODE);
    return -1;
  }

  // get the obs_id of this subobservation
  long this_obs_id = 0;
  if (ascii_header_get(client->header, HEADER_OBS_ID, "%lu", &this_obs_id) == -1)
  {
    multilog(log, LOG_ERR, "dada_dbfits_open(): %s not found in header.\n", HEADER_OBS_ID);
    return -1;
  }

  long this_subobs_id = 0;
  if (ascii_header_get(client->header, HEADER_SUBOBS_ID, "%lu", &this_subobs_id) == -1)
  {
    multilog(log, LOG_ERR, "dada_dbfits_open(): %s not found in header.\n", HEADER_SUBOBS_ID);
    return -1;
  }

  // Sanity check this obs_id
  if (!(this_obs_id > 0))
  {
    multilog(log, LOG_ERR, "dada_dbfits_open(): New %s is not greater than 0.\n", HEADER_OBS_ID);
    return -1;
  }

  int new_obs_id = 0;

  // Check this obs_id against our 'in progress' obsid
  if (ctx->obs_id != this_obs_id || ctx->fits_file_size >= ctx->fits_file_size_limit)
  {
    // We need a new fits file
    if (ctx->obs_id != this_obs_id)
    {
      // Set this flag so we know whats going on later in this function
      new_obs_id = 1;

      if (ctx->fits_ptr != NULL)
      {
        multilog(log, LOG_INFO, "dada_dbfits_open(): New %s detected. Closing %lu, Starting %lu...\n", HEADER_OBS_ID, ctx->obs_id, this_obs_id);
      }
      else
      {
        multilog(log, LOG_INFO, "dada_dbfits_open(): New %s detected. Starting %lu...\n", HEADER_OBS_ID, this_obs_id);
      }
    }
    else
    {
      multilog(log, LOG_INFO, "dada_dbfits_open(): Current file size (%lu bytes) exceeds max size (%lu bytes) of a fits file. Closing %s, Starting new file...\n", ctx->fits_file_size, ctx->fits_file_size_limit, ctx->fits_filename);
    }

    // Close existing fits file (if we have one)
    if (ctx->fits_ptr != NULL)
    {
      if (close_fits(client, &ctx->fits_ptr))
      {
        multilog(log, LOG_ERR, "dada_dbfits_open(): Error closing fits file.\n");
        return -1;
      }

      // Reset current file size
      ctx->fits_file_size = 0;
    }

    if (ctx->obs_id != this_obs_id)
    {
      // If it is a new observation (ctx->obs_id != this_obs_id)
      // But the this_obs_id != this_subobs_id, then it means we are not at the start of an observation and we should skip it
      if (this_obs_id != this_subobs_id)
      {
        multilog(log, LOG_WARNING, "dada_dbfil_open(): Detected an in progress observation (obs_id: %lu / sub_obs_id: %lu). Skipping this observation.\n", this_obs_id, this_subobs_id);
        // Set obs and subobs to 0 so the io and close methods know we have nothing to do
        ctx->obs_id = 0;
        ctx->subobs_id = 0;
      }
      else
      {
        //
        // Do this for new observations only
        //

        // initialise our structure
        ctx->block_open = 0;
        ctx->bytes_read = 0;
        ctx->bytes_written = 0;
        ctx->curr_block = 0;
        ctx->block_number = 0;

        // fits info
        strncpy(ctx->fits_filename, "", PATH_MAX);

        // Set the obsid & sub obsid
        ctx->obs_id = this_obs_id;
        ctx->subobs_id = this_subobs_id;

        // Read in all of the info from the header into our struct
        if (read_dada_header(client))
        {
          // Error processing in header!
          multilog(log, LOG_ERR, "dada_dbfits_open(): Error processing header.\n");
          return -1;
        }

        /*                          */
        /* Sanity check what we got */
        /*                          */
        /* Do have a positive number of inputs (tile * pol) and are they a multiple of 16? */
        if (!(ctx->ninputs_xgpu > 0 && (ctx->ninputs_xgpu % XGPU_INPUT_STRIDE == 0)))
        {
          multilog(log, LOG_ERR, "dada_dbfits_open(): %s is not greater than 0 or a multiple of %d (as required by xGPU).\n", HEADER_NINPUTS_XGPU, XGPU_INPUT_STRIDE);
          return -1;
        }

        /* Coarse channel number needs to be in range 0-255 */
        if (!(ctx->coarse_channel >= 0 && ctx->coarse_channel <= COARSE_CHANNEL_MAX))
        {
          multilog(log, LOG_ERR, "dada_dbfits_open(): %s is not between 0 and %d.\n", HEADER_COARSE_CHANNEL, COARSE_CHANNEL_MAX);
          return -1;
        }

        /* Correlator coarse channel number must be in range 0-23 */
        if (!(ctx->corr_coarse_channel >= 0 && ctx->corr_coarse_channel < CORR_COARSE_CHANNEL_MAX))
        {
          multilog(log, LOG_ERR, "dada_dbfits_open(): %s is not between 0 and %d.\n", HEADER_CORR_COARSE_CHANNEL, CORR_COARSE_CHANNEL_MAX);
          return -1;
        }

        /* ProjectID needs to be less than 255 chars */
        if (!(strlen(ctx->proj_id) <= PROJ_ID_LEN))
        {
          multilog(log, LOG_ERR, "dada_dbfits_open(): %s must be %d characters long.\n", HEADER_PROJ_ID, PROJ_ID_LEN);
          return -1;
        }

        /* Bandwidth in Hz needs to be > 0 */
        if (!(ctx->bandwidth_hz > 0))
        {
          multilog(log, LOG_ERR, "dada_dbfits_open(): %s is not greater than 0.\n", HEADER_BANDWIDTH_HZ);
          return -1;
        }

        /* fsscrunch factor needs to be > 0 */
        if (!(ctx->fscrunch_factor > 0))
        {
          multilog(log, LOG_ERR, "dada_dbfits_open(): %s is not greater than 0.\n", HEADER_FSCRUNCH_FACTOR);
          return -1;
        }

        /* polarisations needs to be > 0 */
        if (!(ctx->npol > 0))
        {
          multilog(log, LOG_ERR, "dada_dbfits_open(): %s is not greater than 0.\n", HEADER_NPOL);
          return -1;
        }

        /* fine channel width must be at least 1hz and at most the bandwidth of a coarse channel */
        if (!(ctx->fine_chan_width_hz >= 1 && ctx->fine_chan_width_hz <= ctx->bandwidth_hz))
        {
          multilog(log, LOG_ERR, "dada_dbfits_open(): %s is not between 1 Hz and %ul Hz.\n", HEADER_FINE_CHAN_WIDTH_HZ, ctx->bandwidth_hz);
          return -1;
        }

        /* We have NFINE_CHAN and we have FINE_CHAN_WIDTH - do these match? */
        if (!((int)(ctx->bandwidth_hz / ctx->nfine_chan) == ctx->fine_chan_width_hz))
        {
          multilog(log, LOG_ERR, "dada_dbfits_open(): %s does not match based on %s and %s.\n", HEADER_FINE_CHAN_WIDTH_HZ, HEADER_BANDWIDTH_HZ, HEADER_NFINE_CHAN);
          return -1;
        }

        /* seconds per sub observation must be > 0 */
        if (!(ctx->secs_per_subobs > 0))
        {
          multilog(log, LOG_ERR, "dada_dbfits_open(): %s is not greater than 0.\n", HEADER_SECS_PER_SUBOBS);
          return -1;
        }

        /* Is exposure time min of 8 secs and a multiple of 8? */
        if (!(ctx->exposure_sec >= ctx->secs_per_subobs && (ctx->exposure_sec % ctx->secs_per_subobs == 0)))
        {
          multilog(log, LOG_ERR, "dada_dbfits_open(): %s is not greater than or equal to %d or a multiple of %d seconds.\n", HEADER_EXPOSURE_SECS, ctx->secs_per_subobs, ctx->secs_per_subobs);
          return -1;
        }

        /* integration time needs to be at least 200ms and at most msec_per_subobs */
        int msec_per_subobs = ctx->secs_per_subobs * 1000;
        if (!(ctx->int_time_msec >= INT_TIME_MSEC_MIN && ctx->int_time_msec <= msec_per_subobs))
        {
          multilog(log, LOG_ERR, "dada_dbfits_open(): %s is not between than %d ms and %d ms.\n", HEADER_INT_TIME_MSEC, msec_per_subobs * 1000, INT_TIME_MSEC_MIN);
          return -1;
        }

        /* transfer size must be > 0*/
        if (!(ctx->transfer_size > 0))
        {
          multilog(log, LOG_ERR, "dada_dbfits_open(): %s is not greater than 0.\n", HEADER_TRANSFER_SIZE);
          return -1;
        }

        /* bits per valye must be at least 8 and a multiple of a byte (8) */
        if (!(ctx->nbit >= 8 && (ctx->nbit % 8 == 0)))
        {
          multilog(log, LOG_ERR, "dada_dbfits_open(): %s is not greater than or equal to 8 or a multiple of 8 bits.\n", HEADER_NBIT);
          return -1;
        }

        /* unix time msec must be between 0 and 999 */
        if (!(ctx->unix_time_msec >= 0 || ctx->unix_time_msec < 1000))
        {
          multilog(log, LOG_ERR, "dada_dbfits_open(): %s must be between 0 and 999 milliseconds.\n", HEADER_UNIXTIME_MSEC);
          return -1;
        }

        // Calculate baselines
        ctx->nbaselines = (ctx->ninputs_xgpu * (ctx->ninputs_xgpu + 2)) / 8;

        //
        // Check transfer size read in from header matches what we expect from the other params
        //

        // Should be 4 bytes per float (32 bits) x2 for r,i
        int bytes_per_float = ctx->nbit / 8;
        int bytes_per_complex = bytes_per_float * 2;

        // Integrations per sub obs
        ctx->no_of_integrations_per_subobs = (ctx->secs_per_subobs * 1000) / ctx->int_time_msec;

        // One fine channel = pol*pol*bytes_per_complex*baselines
        ctx->expected_transfer_size_of_one_fine_channel = ctx->npol * ctx->npol * bytes_per_complex * ctx->nbaselines;

        // weights (in one integration) = (pol * pol * bytes per weight * baselines)
        ctx->expected_transfer_size_of_weights = ctx->npol * ctx->npol * bytes_per_float * ctx->nbaselines;

        // one fine chan * number of fine channels
        ctx->expected_transfer_size_of_integration = ctx->expected_transfer_size_of_one_fine_channel * ctx->nfine_chan;

        // one integration + weights
        ctx->expected_transfer_size_of_integration_plus_weights = ctx->expected_transfer_size_of_integration + ctx->expected_transfer_size_of_weights;

        // one integration * number of integrations
        ctx->expected_transfer_size_of_subobs = ctx->expected_transfer_size_of_integration * ctx->no_of_integrations_per_subobs;

        // one sub obs + (weights * number of integrations per sub obs)
        ctx->expected_transfer_size_of_subobs_plus_weights = ctx->expected_transfer_size_of_subobs + (ctx->expected_transfer_size_of_weights * ctx->no_of_integrations_per_subobs);

        // The number of bytes should never exceed transfer size
        if (ctx->expected_transfer_size_of_subobs_plus_weights > ctx->transfer_size)
        {
          multilog(log, LOG_ERR, "dada_dbfits_open(): %s provided in header (%lu bytes) is not large enough for a subobservation size of (%lu bytes).\n", HEADER_TRANSFER_SIZE, ctx->transfer_size, ctx->expected_transfer_size_of_subobs_plus_weights);
          return -1;
        }

        // Also confirm that the integration size can fit into the ringbuffer size
        if (ctx->expected_transfer_size_of_integration_plus_weights > ctx->block_size)
        {
          multilog(log, LOG_ERR, "dada_dbfits_open(): Ring buffer block size (%lu bytes) is less than the calculated size of an integration from header parameters (%lu bytes).\n", ctx->block_size, ctx->expected_transfer_size_of_integration_plus_weights);
          return -1;
        }

        // Reset the filenumber
        ctx->fits_file_number = 0;
      }
    }
    else
    {
      // Do this only if we exceeded the size of a fits file and need a new file
      ctx->fits_file_number++;
    }

    /* Create fits file for output                                */
    /* Work out the name of the file using the UTC START          */
    /* Convert the UTC_START from the header format: YYYY-MM-DD-hh:mm:ss into YYYYMMDDhhmmss  */
    int year, month, day, hour, minute, second;
    sscanf(ctx->utc_start, "%d-%d-%d-%d:%d:%d", &year, &month, &day, &hour, &minute, &second);

    /* Make a new filename- oooooooooo_YYYYMMDDhhmmss_chCCC_FFF.fits */
    snprintf(ctx->fits_filename, PATH_MAX, "%s/%ld_%04d%02d%02d%02d%02d%02d_ch%03d_%03d.fits", ctx->destination_dir, ctx->obs_id, year, month, day, hour, minute, second, ctx->coarse_channel, ctx->fits_file_number);

    if (create_fits(client, &ctx->fits_ptr, ctx->fits_filename))
    {
      multilog(log, LOG_ERR, "dada_dbfits_open(): Error creating new fits file.\n");
      return -1;
    }

    // Reset file size
    ctx->fits_file_size = 0;
  }

  /* This is a continuation of an existing observation */
  if (new_obs_id == 0)
  {
    multilog(log, LOG_INFO, "dada_dbfits_open(): continuing %lu...\n", ctx->obs_id);

    /* Get the duration */
    int new_duration_sec = 0;
    if (ascii_header_get(client->header, HEADER_EXPOSURE_SECS, "%i", &new_duration_sec) == -1)
    {
      multilog(log, LOG_ERR, "dada_dbfits_open(): %s not found in header.\n", HEADER_EXPOSURE_SECS);
      return -1;
    }

    /* has the duration changed? */
    if (new_duration_sec != ctx->exposure_sec)
    {
      multilog(log, LOG_INFO, "dada_dbfits_open(): %s has changed from %d sec to %d sec.\n", HEADER_EXPOSURE_SECS, ctx->exposure_sec, new_duration_sec);
    }

    /* Get the offset */
    int new_obs_offset_sec = 0;
    if (ascii_header_get(client->header, HEADER_OBS_OFFSET, "%i", &new_obs_offset_sec) == -1)
    {
      multilog(log, LOG_ERR, "dada_dbfits_open(): %s not found in header.\n", HEADER_OBS_OFFSET);
      return -1;
    }

    /* has the offset incremented? */
    if (new_obs_offset_sec == ctx->obs_offset)
    {
      multilog(log, LOG_ERR, "dada_dbfits_open(): %s is the same as the previous subobservation (%d == %d).\n", HEADER_OBS_OFFSET, ctx->obs_offset, new_obs_offset_sec);
      return -1;
    }
    else if (new_obs_offset_sec < ctx->obs_offset)
    {
      multilog(log, LOG_ERR, "dada_dbfits_open(): %s is less than the previous observation (%d < %d).\n", HEADER_OBS_OFFSET, ctx->obs_offset, new_obs_offset_sec);
      return -1;
    }
    else if (new_obs_offset_sec - ctx->obs_offset != ctx->secs_per_subobs)
    {
      multilog(log, LOG_ERR, "dada_dbfits_open(): %s did not increase by %d seconds (it was: %d).\n", HEADER_OBS_OFFSET, ctx->secs_per_subobs, new_obs_offset_sec - ctx->obs_offset);
      return -1;
    }
    else
    {
      multilog(log, LOG_INFO, "dada_dbfits_open(): %s incremented from %d sec to %d sec.\n", HEADER_OBS_OFFSET, ctx->exposure_sec, new_duration_sec);
    }

    /* update new values */
    ctx->exposure_sec = new_duration_sec;
    ctx->obs_offset = new_obs_offset_sec;
  }

  multilog(log, LOG_INFO, "dada_dbfits_open(): completed\n");

  return EXIT_SUCCESS;
}

/**
 * 
 *  @brief This is the function psrdada calls when we have new data to read.
 *         NOTE: this method reads an entire block of PSRDADA ringbuffer. The ringbuffer is sized to be the MAX
 *         possible, as it cannot be resized at runtime. Therefore most times, the correlator mode will be producing
 *         less data than this buffer can contain, so we'll need to only get the first x bytes from the buffer.
 *         (Where x is the bytes needded for that correlator mode + 1 extra finechannel for weights)
 * 
 *  @param[in] client A pointer to the dada_client_t object.
 *  @param[in] buffer The pointer to the data in the ringbuffer we are about to read.
 *  @param[in] bytes The number of bytes that are available to be read from the ringbuffer.
 *  @returns the number of bytes read or -1 if there was an error.
 */
int64_t dada_dbfits_io(dada_client_t *client, void *buffer, uint64_t bytes)
{
  assert(client != 0);
  dada_db_s *ctx = (dada_db_s *)client->context;
  multilog_t *log = (multilog_t *)ctx->log;

  // If we're processing an observation...
  if (is_mwax_mode_correlator(ctx->mode) == 1)
  {
    // Check if we are actually processing an obs or just skipping it
    if (ctx->obs_id != 0)
    {
      uint64_t written = 0;
      uint64_t to_write = bytes;
      uint64_t wrote = 0;

      multilog(log, LOG_DEBUG, "dada_dbfits_io(): Processing block %d.\n", ctx->block_number);

      multilog(log, LOG_INFO, "dada_dbfits_io(): Writing %d of %d bytes into new image HDU; Marker = %d.\n", ctx->expected_transfer_size_of_integration, bytes, ctx->obs_marker_number);

      // Write HDU here!
      float *ptr_data = (float *)buffer;

      // Remove the weights from the byte count
      // Remove any left over space from the byte count too
      uint64_t visibility_hdu_bytes = ctx->expected_transfer_size_of_integration;
      uint64_t weights_hdu_bytes = ctx->expected_transfer_size_of_weights;

      // Create the visibility HDU in the FITS file
      if (create_fits_visibilities_imghdu(client, ctx->fits_ptr, ctx->unix_time, ctx->unix_time_msec, ctx->obs_marker_number,
                                          ctx->nbaselines, ctx->nfine_chan, ctx->npol, ptr_data, visibility_hdu_bytes))
      {
        // Error!
        multilog(log, LOG_ERR, "dada_dbfits_io(): Error Writing into new visibility image HDU.\n");
        return -1;
      }
      else
      {
        // Increment the data buffer pointer to skip the "data" so we point at the weights
        float *ptr_weights = ptr_data + (visibility_hdu_bytes / sizeof(float));

        // Now write the weights HDU
        if (create_fits_weights_imghdu(client, ctx->fits_ptr, ctx->unix_time, ctx->unix_time_msec, ctx->obs_marker_number,
                                       ctx->nbaselines, ctx->npol, ptr_weights, weights_hdu_bytes))
        {
          // Error!
          multilog(log, LOG_ERR, "dada_dbfits_io(): Error Writing into new weights image HDU.\n");
          return -1;
        }
        else
        {
          wrote = to_write;
          written += wrote;
          ctx->fits_file_size = ctx->fits_file_size + visibility_hdu_bytes + weights_hdu_bytes;

          //multilog(log, LOG_INFO, "dada_dbfits_io(): Current fits file size: %ld / %ld\n", ctx->fits_file_size, ctx->fits_file_size_limit);

          ctx->obs_marker_number += 1; // Increment the marker number

          // Increment the UNIX time marker by: int_time_msec
          ctx->unix_time_msec += ctx->int_time_msec;

          while (ctx->unix_time_msec >= 1000)
          {
            ctx->unix_time += 1;
            ctx->unix_time_msec -= 1000;
          }
        }
      }

      ctx->block_number += 1;

      // Check to see if we are the last expected block
      if (ctx->block_number % ctx->no_of_integrations_per_subobs == 0)
      {
        // Write out stats
        if (dump_autocorrelation_stats(client, ptr_data) != EXIT_SUCCESS)
        {
          // Error!
          multilog(log, LOG_ERR, "dada_dbfits_io(): Error Writing autocorrelation stats dump.\n");
          return -1;
        }
      }

      ctx->bytes_written += written;
    }

    return bytes;
  }
  else
  {
    multilog(log, LOG_WARNING, "dada_dbfits_io(): Unknown mode %s; (ignoring).\n", ctx->mode);
    return 0;
  }
}

/**
 * 
 *  @brief This will dump out this integrations autocorrelations to a file
 *  @param[in] client A pointer to the dada_client_t object.
 *  @param[in] buffer The pointer to the data in the ringbuffer we are about to read.
 *  @param[in] bytes The number of bytes that the visibilities take up
 *  @returns SUCCESS or EXIT_FAILURE
 */
int dump_autocorrelation_stats(dada_client_t *client, void *ptr_data)
{
  assert(client != 0);
  dada_db_s *ctx = (dada_db_s *)client->context;
  multilog_t *log = (multilog_t *)ctx->log;

  int baseline = 0;
  int baseline_offset = 0;
  int chan_offset = 0;
  int xx_r_offset = 0;
  int yy_r_offset = 6;
  int ntiles = ctx->ninputs_xgpu / 2;

  char stats_filename_tmp[PATH_MAX];

  float *data = (float *)ptr_data;

  // Generate filename
  // We begin with a tmp filename then rename once we have completed writing
  snprintf(ctx->stats_filename, PATH_MAX - 4, "%s/%ld_autos.txt", ctx->stats_dir, ctx->subobs_id);
  snprintf(stats_filename_tmp, PATH_MAX, "%s.tmp", ctx->stats_filename);

  multilog(log, LOG_INFO, "dump_autocorrelation_stats(): Openning autocorrelation stats dump temp file for writing %s...\n", stats_filename_tmp);

  FILE *fp = fopen(stats_filename_tmp, "wb"); // Change to "w" for ascii output- useful for debugging

  if (fp == NULL)
  {
    multilog(log, LOG_ERR, "dump_autocorrelation_stats(): Error openning autocorrelation stats dump for writing %s: %s\n", ctx->stats_filename, strerror(errno));
    return EXIT_FAILURE;
  }

  for (int i = 0; i < ntiles; i++)
  {
    for (int j = i; j < ntiles; j++)
    {
      // Autocorrelations only
      if (i == j)
      {
        // Uncomment this for debug: fprintf(fp, "Baseline: %d x %d (%d): ", i, j, baseline);

        // Determine where the channel starts
        baseline_offset = baseline * ctx->nfine_chan * (ctx->npol * ctx->npol) * 2;

        // Iterate all the fine channels for this baseline
        for (int ch = 0; ch < ctx->nfine_chan; ch++)
        {
          chan_offset = (ch * (ctx->npol * ctx->npol) * 2);

          // Only dump xx_r and yy_r
          fwrite(&data[baseline_offset + chan_offset + xx_r_offset], sizeof(float), 1, fp);
          fwrite(&data[baseline_offset + chan_offset + yy_r_offset], sizeof(float), 1, fp);

          //Uncomment for debug
          //fprintf(fp, "(ch%d)xxr=%.5f,yy_r=%.5f", ch, data[baseline_offset+chan_offset+xx_r_offset], data[baseline_offset+chan_offset+yy_r_offset]);
        }
      }

      baseline = baseline + 1;
    }
  }

  // Close file
  if (fp != NULL)
    fclose(fp);

  multilog(log, LOG_INFO, "dump_autocorrelation_stats(): Finished writing. Renaming temp file %s to %s.\n", stats_filename_tmp, ctx->stats_filename);

  int rename_retval = rename(stats_filename_tmp, ctx->stats_filename);

  if (rename_retval == 0)
  {
    multilog(log, LOG_INFO, "dump_autocorrelation_stats(): completed\n", ctx->stats_filename);
  }
  else
  {
    perror("Error: ");
    multilog(log, LOG_PERROR, "dump_autocorrelation_stats(): Could not rename file %s to %s (Error code %d).\n", stats_filename_tmp, ctx->stats_filename, rename_retval);
  }

  return EXIT_SUCCESS;
}

/**
 * 
 *  @brief This is called when we are reading a sub block of an 8 second sub-observation.
 *  @param[in] client A pointer to the dada_client_t object.
 *  @param[in] buffer The pointer to the data in the ringbuffer we are about to read.
 *  @param[in] bytes The number of bytes that are available to be read from the ringbuffer
 *  @param[in] block_id The block number (id) of the block we are reading.
 *  @returns the number of bytes read, or -1 if there was an error.
 */
int64_t dada_dbfits_io_block(dada_client_t *client, void *buffer, uint64_t bytes, uint64_t block_id)
{
  assert(client != 0);
  dada_db_s *ctx = (dada_db_s *)client->context;
  multilog_t *log = (multilog_t *)ctx->log;

  if (is_mwax_mode_correlator(ctx->mode) == 1)
  {
    multilog(log, LOG_INFO, "dada_dbfits_io_block(): Processing block id %llu\n", block_id);

    return dada_dbfits_io(client, buffer, bytes);
  }
  else
  {
    multilog(log, LOG_WARNING, "dada_dbfits_io_block(): Unknown mode %s; (ignoring).\n", ctx->mode);
    return bytes;
  }
}

/**
 * 
 *  @brief This is called at the end of each new 8 second sub-observation.
 *         We need check if the current observation duration has changed (shortened) from what we expected.
 *  @param[in] client A pointer to the dada_client_t object.
 *  @param[in] bytes_written The number of bytes that psrdada has written for this entire 8 second subobservation.
 *  @returns EXIT_SUCCESS on success, or -1 if there was an error.
 */
int dada_dbfits_close(dada_client_t *client, uint64_t bytes_written)
{
  assert(client != 0);
  dada_db_s *ctx = (dada_db_s *)client->context;

  multilog_t *log = (multilog_t *)client->log;
  multilog(log, LOG_INFO, "dada_dbfits_close(bytes_written=%lu): Started.\n", bytes_written);

  int do_close_fits = 0;

  // If we're still in CAPTURE mode...
  if (is_mwax_mode_correlator(ctx->mode) == 1)
  {
    // Check if we are actually processing an obs or just skipping it
    if (ctx->obs_id != 0)
    {
      // Some sanity checks:
      int current_duration = (int)((float)(ctx->obs_marker_number) * ((float)ctx->int_time_msec / 1000.0));

      // We should be at a marker which when multiplied by int_time should be a multuple of ctx->obs_secs_per_subobs (8 seconds nominally).
      multilog(log, LOG_INFO, "dada_dbfits_close(): Checking duration based on current marker %d vs obs duration %d.\n", current_duration, ctx->exposure_sec);

      if (current_duration % ctx->secs_per_subobs != 0)
      {
        multilog(log, LOG_ERR, "dada_dbfits_close(): Error, the dada ringbuffer closed at %d secs before we got all %d secs of data! (marker=%d)\n", current_duration, ctx->secs_per_subobs, ctx->obs_marker_number);
        return -1;
      }

      //
      // TODO: Check with the metabin for info about this observation. Has the duration changed?
      //
      int new_duration = ctx->exposure_sec; //TODO: Fix me! This is a placeholder

      if (ctx->exposure_sec != new_duration)
      {
        multilog(log, LOG_INFO, "dada_dbfits_close(): Observation has been cut short. Old duration was %d, new duration is %d.\n", ctx->exposure_sec, new_duration);
        ctx->exposure_sec = new_duration; // TODO: put new duration from metabin here
      }

      // Did we hit the end of an obs
      if (current_duration == ctx->exposure_sec)
      {
        do_close_fits = 1;
      }
    }
    else
    {
      multilog(log, LOG_WARNING, "dada_dbfits_close(): Unknown mode %s; (ignoring).\n", ctx->mode);
      do_close_fits = 1;
    }

    if (do_close_fits == 1)
    {
      // Observation ends NOW! It got cut short, or we naturally are at the end of the observation
      // Close existing fits file (if we have one)
      if (ctx->fits_ptr != NULL)
      {
        if (close_fits(client, &ctx->fits_ptr))
        {
          multilog(log, LOG_ERR, "dada_dbfits_close(): Error closing fits file.\n");
          return -1;
        }
      }
    }
  }

  multilog(log, LOG_INFO, "dada_dbfits_close(): completed\n");

  return EXIT_SUCCESS;
}

/**
 * 
 *  @brief This reads a PSRDADA header and populates our context structure and dumps the contents into a debug log
 *  @param[in] client A pointer to the dada_client_t object. 
 *  @returns EXIT_SUCCESS on success, or -1 if there was an error.
 */
int read_dada_header(dada_client_t *client)
{
  // Reset and read everything except for obs_id and subobs_id
  assert(client != 0);
  dada_db_s *ctx = (dada_db_s *)client->context;

  assert(ctx->log != 0);
  multilog_t *log = (multilog_t *)ctx->log;

  ctx->populated = 0;
  strncpy(ctx->utc_start, "", UTC_START_LEN);
  ctx->obs_offset = 0;
  ctx->nbit = 0;
  ctx->npol = 0;
  ctx->ninputs_xgpu = 0;
  ctx->int_time_msec = 0;
  ctx->transfer_size = 0;
  strncpy(ctx->proj_id, "", PROJ_ID_LEN);
  ctx->exposure_sec = 0;
  ctx->coarse_channel = 0;
  ctx->corr_coarse_channel = 0;
  ctx->secs_per_subobs = 0;
  ctx->unix_time = 0;
  ctx->unix_time_msec = 0;
  ctx->fine_chan_width_hz = 0;
  ctx->nfine_chan = 0;
  ctx->bandwidth_hz = 0;
  ctx->fscrunch_factor = 0;

  strncpy(ctx->mwax_u2s_version, "Unknown", MWAX_VERSION_STRING_LEN);
  strncpy(ctx->mwax_db2correlate2db_version, "Unknown", MWAX_VERSION_STRING_LEN);

  strncpy(ctx->multicast_ip, "", IP_AS_STRING_LEN);
  ctx->multicast_port = 0;

  ctx->nbaselines = 0;
  ctx->obs_marker_number = 0;

  if (ascii_header_get(client->header, HEADER_POPULATED, "%i", &ctx->populated) == -1)
  {
    multilog(log, LOG_ERR, "read_dada_header(): %s not found in header.\n", HEADER_POPULATED);
    return -1;
  }

  if (ascii_header_get(client->header, HEADER_UTC_START, "%s", &ctx->utc_start) == -1)
  {
    multilog(log, LOG_ERR, "read_dada_header(): %s not found in header.\n", HEADER_UTC_START);
    return -1;
  }

  if (ascii_header_get(client->header, HEADER_OBS_OFFSET, "%i", &ctx->obs_offset) == -1)
  {
    multilog(log, LOG_ERR, "read_dada_header(): %s not found in header.\n", HEADER_OBS_OFFSET);
    return -1;
  }

  if (ascii_header_get(client->header, HEADER_NBIT, "%i", &ctx->nbit) == -1)
  {
    multilog(log, LOG_ERR, "read_dada_header(): %s not found in header.\n", HEADER_NBIT);
    return -1;
  }

  if (ascii_header_get(client->header, HEADER_NPOL, "%i", &ctx->npol) == -1)
  {
    multilog(log, LOG_ERR, "read_dada_header(): %s not found in header.\n", HEADER_NPOL);
    return -1;
  }

  if (ascii_header_get(client->header, HEADER_NINPUTS_XGPU, "%i", &ctx->ninputs_xgpu) == -1)
  {
    multilog(log, LOG_ERR, "read_dada_header(): %s not found in header.\n", HEADER_NINPUTS_XGPU);
    return -1;
  }

  if (ascii_header_get(client->header, HEADER_INT_TIME_MSEC, "%i", &ctx->int_time_msec) == -1)
  {
    multilog(log, LOG_ERR, "read_dada_header(): %s not found in header.\n", HEADER_INT_TIME_MSEC);
    return -1;
  }

  if (ascii_header_get(client->header, HEADER_TRANSFER_SIZE, "%lu", &ctx->transfer_size) == -1)
  {
    multilog(log, LOG_ERR, "read_dada_header(): %s not found in header.\n", HEADER_TRANSFER_SIZE);
    return -1;
  }

  if (ascii_header_get(client->header, HEADER_PROJ_ID, "%s", &ctx->proj_id) == -1)
  {
    multilog(log, LOG_ERR, "read_dada_header(): %s not found in header.\n", HEADER_PROJ_ID);
    return -1;
  }

  if (ascii_header_get(client->header, HEADER_EXPOSURE_SECS, "%i", &ctx->exposure_sec) == -1)
  {
    multilog(log, LOG_ERR, "read_dada_header(): %s not found in header.\n", HEADER_EXPOSURE_SECS);
    return -1;
  }

  if (ascii_header_get(client->header, HEADER_COARSE_CHANNEL, "%i", &ctx->coarse_channel) == -1)
  {
    multilog(log, LOG_ERR, "read_dada_header(): %s not found in header.\n", HEADER_COARSE_CHANNEL);
    return -1;
  }

  if (ascii_header_get(client->header, HEADER_CORR_COARSE_CHANNEL, "%i", &ctx->corr_coarse_channel) == -1)
  {
    multilog(log, LOG_ERR, "read_dada_header(): %s not found in header.\n", HEADER_CORR_COARSE_CHANNEL);
    return -1;
  }

  if (ascii_header_get(client->header, HEADER_SECS_PER_SUBOBS, "%i", &ctx->secs_per_subobs) == -1)
  {
    multilog(log, LOG_ERR, "read_dada_header(): %s not found in header.\n", HEADER_SECS_PER_SUBOBS);
    return -1;
  }

  if (ascii_header_get(client->header, HEADER_UNIXTIME, "%lu", &ctx->unix_time) == -1)
  {
    multilog(log, LOG_ERR, "read_dada_header(): %s not found in header.\n", HEADER_UNIXTIME);
    return -1;
  }

  if (ascii_header_get(client->header, HEADER_UNIXTIME_MSEC, "%i", &ctx->unix_time_msec) == -1)
  {
    multilog(log, LOG_ERR, "read_dada_header(): %s not found in header.\n", HEADER_UNIXTIME_MSEC);
    return -1;
  }

  if (ascii_header_get(client->header, HEADER_FINE_CHAN_WIDTH_HZ, "%i", &ctx->fine_chan_width_hz) == -1)
  {
    multilog(log, LOG_ERR, "read_dada_header(): %s not found in header.\n", HEADER_FINE_CHAN_WIDTH_HZ);
    return -1;
  }

  if (ascii_header_get(client->header, HEADER_NFINE_CHAN, "%i", &ctx->nfine_chan) == -1)
  {
    multilog(log, LOG_ERR, "read_dada_header(): %s not found in header.\n", HEADER_NFINE_CHAN);
    return -1;
  }

  if (ascii_header_get(client->header, HEADER_BANDWIDTH_HZ, "%i", &ctx->bandwidth_hz) == -1)
  {
    multilog(log, LOG_ERR, "read_dada_header(): %s not found in header.\n", HEADER_BANDWIDTH_HZ);
    return -1;
  }

  if (ascii_header_get(client->header, HEADER_FSCRUNCH_FACTOR, "%i", &ctx->fscrunch_factor) == -1)
  {
    multilog(log, LOG_ERR, "read_dada_header(): %s not found in header.\n", HEADER_FSCRUNCH_FACTOR);
    return -1;
  }

  if (ascii_header_get(client->header, HEADER_MC_IP, "%s", &ctx->multicast_ip) == -1)
  {
    multilog(log, LOG_ERR, "read_dada_header(): %s not found in header.\n", HEADER_MC_IP);
    return -1;
  }

  if (ascii_header_get(client->header, HEADER_MC_PORT, "%i", &ctx->multicast_port) == -1)
  {
    multilog(log, LOG_ERR, "read_dada_header(): %s not found in header.\n", HEADER_MC_PORT);
    return -1;
  }

  // for now if we don't get version info from other mwax components it is just a warning
  if (ascii_header_get(client->header, HEADER_MWAX_U2S_VERSION, "%s", &ctx->mwax_u2s_version) == -1)
  {
    multilog(log, LOG_WARNING, "read_dada_header(): %s not found in header.\n", HEADER_MWAX_U2S_VERSION);
  }

  if (ascii_header_get(client->header, HEADER_MWAX_DB2CORRELATE2DB_VERSION, "%s", &ctx->mwax_db2correlate2db_version) == -1)
  {
    multilog(log, LOG_WARNING, "read_dada_header(): %s not found in header.\n", HEADER_MWAX_DB2CORRELATE2DB_VERSION);
  }

  // Output what we found in the header
  multilog(log, LOG_INFO, "Populated?:               %s\n", (ctx->populated == 1 ? "yes" : "no"));
  multilog(log, LOG_INFO, "Obs Id:                   %lu\n", ctx->obs_id);
  multilog(log, LOG_INFO, "Subobs Id:                %lu\n", ctx->subobs_id);
  multilog(log, LOG_INFO, "Offset:                   %d sec\n", ctx->obs_offset);
  multilog(log, LOG_INFO, "Mode:                     %s\n", ctx->mode);
  multilog(log, LOG_INFO, "Start time (UTC):         %s\n", ctx->utc_start);
  multilog(log, LOG_INFO, "Correlator freq res:      %0.1f kHz\n", (float)ctx->fine_chan_width_hz / 1000.0f);
  multilog(log, LOG_INFO, "Correlator int time:      %0.2f sec\n", (float)ctx->int_time_msec / 1000.0f);
  multilog(log, LOG_INFO, "No fine chans per coarse: %d\n", ctx->nfine_chan);
  multilog(log, LOG_INFO, "Coarse channel width:     %d kHz\n", ctx->bandwidth_hz / 1000);
  multilog(log, LOG_INFO, "Bits per real/imag:       %d\n", ctx->nbit);
  multilog(log, LOG_INFO, "Polarisations:            %d\n", ctx->npol);
  multilog(log, LOG_INFO, "Tiles:                    %d\n", ctx->ninputs_xgpu / 2);
  multilog(log, LOG_INFO, "Project Id:               %s\n", ctx->proj_id);
  multilog(log, LOG_INFO, "Duration:                 %d sec\n", ctx->exposure_sec);
  multilog(log, LOG_INFO, "Coarse channel no.:       %d\n", ctx->coarse_channel);
  multilog(log, LOG_INFO, "Corr Coarse channel no.:  %d\n", ctx->corr_coarse_channel);
  multilog(log, LOG_INFO, "Duration of subobs:       %d sec\n", ctx->secs_per_subobs);
  multilog(log, LOG_INFO, "UNIX time of subobs:      %lu\n", ctx->unix_time);
  multilog(log, LOG_INFO, "UNIX milliseconds:        %d msec\n", ctx->unix_time_msec);
  multilog(log, LOG_INFO, "Size of subobservation:   %lu bytes\n", ctx->transfer_size);
  multilog(log, LOG_INFO, "Multicast IP:             %s\n", ctx->multicast_ip);
  multilog(log, LOG_INFO, "Multicast Port:           %d\n", ctx->multicast_port);
  multilog(log, LOG_INFO, "mwax_u2s version:         %s\n", ctx->mwax_u2s_version);
  multilog(log, LOG_INFO, "mwax_db2corr2db version:  %s\n", ctx->mwax_db2correlate2db_version);

  // Update health info
  if (set_health(STATUS_RUNNING, ctx->obs_id, ctx->subobs_id) != EXIT_SUCCESS)
  {
    multilog(log, LOG_ERR, "read_dada_header(): Error setting health data");
    return -1;
  }

  return EXIT_SUCCESS;
}
