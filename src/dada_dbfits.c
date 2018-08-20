/**
 * @file dada_dbfits.c
 * @author Greg Sleap
 * @date 23 May 2018
 * @brief This is the code that drives the ring buffers
 *
 */
#include "utils.h"
#include "dada_dbfits.h"

/**
 * 
 *  @brief This is called at the begininning of each new 8 second sub-observation.
 *         We need check if we are in a new fits file or continuing the existing one.
 *  @param[in] client A pointer to the dada_client_t object.
 *  @returns EXIT_SUCCESS on success, or -1 if there was an error. 
 */
int dada_dbfits_open(dada_client_t* client)
{
  assert(client != 0);
  dada_db_s* ctx = (dada_db_s*) client->context;

  multilog_t *log = (multilog_t *) client->log;  
  multilog(log, LOG_INFO, "dada_db_open(): extracting params from dada header\n");

  // get the obs_id of this subobservation
  long this_obs_id = 0;
  if (ascii_header_get(client->header, "OBS_ID", "%lu", &this_obs_id) == -1)
  {
    multilog(log, LOG_ERR, "dada_db_open(): OBS_ID not found in header.\n");
    return -1;
  }

  // Sanity check this obs_id
  if (!(this_obs_id > 0))
  {
    multilog(log, LOG_ERR, "dada_db_open(): New OBS_ID is not greater than 0.\n");
    return -1;
  }

  // Check this obs_id against our 'in progress' obsid  
  if (ctx->obs_id != this_obs_id || ctx->bytes_written >= FITS_SIZE_CUTOFF_BYTES)
  {
    // We need a new fits file
    if (ctx->obs_id != this_obs_id)
    {
      multilog(log, LOG_INFO, "dada_db_open(): New OBS_ID detected. Closing %lu, Starting %lu...\n", ctx->obs_id, this_obs_id);
    }
    else
    {
      multilog(log, LOG_INFO, "dada_db_open(): Exceeded max size %lu of a fits file. Closing %s, Starting new file...\n", FITS_SIZE_CUTOFF_BYTES, ctx->fits_filename);
    }

    // Close existing fits file (if we have one)    
    if (ctx->fits_ptr != NULL)
    {
      if (close_fits(&ctx->fits_ptr)) 
      {
        multilog(log, LOG_ERR,"dada_db_open(): Error closing fits file.\n");
        return -1;
      }
    }
    
    if (ctx->obs_id != this_obs_id)
    {
      //
      // Do this for new observations only
      //

      // initialise our structure
      ctx->block_open = 0;
      ctx->bytes_read = 0;
      ctx->bytes_written = 0;
      ctx->curr_block = 0;
      ctx->block_number= 0;

      // fits info  
      strncpy(ctx->fits_filename, "", PATH_MAX);

      // obs info
      ctx->obs_id = this_obs_id;
      strncpy(ctx->utc_start, "", UTC_START_LEN);
      ctx->obs_duration_sec = 0;
      ctx->obs_channel_number = 0;
      strncpy(ctx->obs_proj_id, "", PROJ_ID_LEN);
      ctx->obs_tiles = 0;
      ctx->obs_bandwidth_khz = 0;
      ctx->obs_pols = 0;
      ctx->obs_freq_res_khz = 0;
      ctx->obs_int_time_msec = 0;
      ctx->obs_subobs_sec = 0;
      ctx->transfer_size_bytes = 0;
      ctx->obs_marker_number = 0;
      ctx->obs_offset_sec = 0;
      ctx->nbit = 0;
      
      if (ascii_header_get(client->header, "UTC_START", "%s", &ctx->utc_start) == -1)
      {
        multilog(log, LOG_ERR, "dada_db_open(): UTC_START not found in header.\n");
        return -1;
      }

      if (ascii_header_get(client->header, "OBS_DURATION", "%i", &ctx->obs_duration_sec) == -1)
      {
        multilog(log, LOG_ERR, "dada_db_open(): OBS_DURATION not found in header.\n");
        return -1;
      }

      if (ascii_header_get(client->header, "OBS_CHANNEL", "%i", &ctx->obs_channel_number) == -1)
      {
        multilog(log, LOG_ERR, "dada_db_open(): OBS_CHANNEL not found in header.\n");
        return -1;
      }

      if (ascii_header_get(client->header, "OBS_PROJ_ID", "%s", &ctx->obs_proj_id) == -1)
      {
        multilog(log, LOG_ERR, "dada_db_open(): OBS_PROJ_ID not found in header.\n");
        return -1;
      }

      if (ascii_header_get(client->header, "OBS_BANDWIDTH", "%i", &ctx->obs_bandwidth_khz) == -1)
      {
        multilog(log, LOG_ERR, "dada_db_open(): OBS_BANDWIDTH not found in header.\n");
        return -1;
      }

      if (ascii_header_get(client->header, "OBS_TILES", "%i", &ctx->obs_tiles) == -1)
      {
        multilog(log, LOG_ERR, "dada_db_open(): OBS_TILES not found in header.\n");
        return -1;
      }

      if (ascii_header_get(client->header, "OBS_POLS", "%i", &ctx->obs_pols) == -1)
      {
        multilog(log, LOG_ERR, "dada_db_open(): OBS_POLS not found in header.\n");
        return -1;
      }

      if (ascii_header_get(client->header, "OBS_FREQ_RES", "%i", &ctx->obs_freq_res_khz) == -1)
      {
        multilog(log, LOG_ERR, "dada_db_open(): OBS_FREQ_RES not found in header.\n");
        return -1;
      }

      if (ascii_header_get(client->header, "OBS_INT_TIME", "%i", &ctx->obs_int_time_msec) == -1)
      {
        multilog(log, LOG_ERR, "dada_db_open(): OBS_INT_TIME not found in header.\n");
        return -1;
      }

      if (ascii_header_get(client->header, "OBS_SECS_PER_SUBOBS", "%i", &ctx->obs_subobs_sec) == -1)
      {
        multilog(log, LOG_ERR, "dada_db_open(): OBS_SECS_PER_SUBOBS not found in header.\n");
        return -1;
      }

      if (ascii_header_get(client->header, "TRANSFER_SIZE", "%lu", &ctx->transfer_size_bytes) == -1)
      {
        multilog(log, LOG_ERR, "dada_db_open(): TRANSFER_SIZE not found in header.\n");
        return -1;
      }

      if (ascii_header_get(client->header, "NBIT", "%i", &ctx->nbit) == -1)
      {
        multilog(log, LOG_ERR, "dada_db_open(): NBIT not found in header.\n");
        return -1;
      }

      if (ascii_header_get(client->header, "UNIX_TIME", "%lu", &ctx->unix_time) == -1)
      {
        multilog(log, LOG_ERR, "dada_db_open(): UNIX_TIME not found in header.\n");
        return -1;
      }

      if (ascii_header_get(client->header, "UNIX_TIME_MSEC", "%i", &ctx->unix_time_msec) == -1)
      {
        multilog(log, LOG_ERR, "dada_db_open(): UNIX_TIME_MSEC not found in header.\n");
        return -1;
      }

      // Output what we found in the header
      multilog(log, LOG_INFO, "OBS_ID:               %lu\n", ctx->obs_id);
      multilog(log, LOG_INFO, "UTC_START:            %s\n", ctx->utc_start);
      multilog(log, LOG_INFO, "OBS_DURATION:         %d sec\n", ctx->obs_duration_sec);
      multilog(log, LOG_INFO, "OBS_CHANNEL:          %d\n", ctx->obs_channel_number);
      multilog(log, LOG_INFO, "OBS_PROJ_ID:          %s\n", ctx->obs_proj_id);
      multilog(log, LOG_INFO, "OBS_TILES:            %d\n", ctx->obs_tiles);
      multilog(log, LOG_INFO, "OBS_BANDWIDTH:        %d kHz\n", ctx->obs_bandwidth_khz);
      multilog(log, LOG_INFO, "OBS_POLS:             %d\n", ctx->obs_pols);
      multilog(log, LOG_INFO, "OBS_FREQ_RES:         %d kHz\n", ctx->obs_freq_res_khz);
      multilog(log, LOG_INFO, "OBS_INT_TIME:         %d msec\n", ctx->obs_int_time_msec);        
      multilog(log, LOG_INFO, "TRANSFER_SIZE:        %lu bytes\n", ctx->transfer_size_bytes);
      multilog(log, LOG_INFO, "NBIT:                 %d\n", ctx->nbit);
      multilog(log, LOG_INFO, "UNIX_TIME:            %lu\n", ctx->unix_time);
      multilog(log, LOG_INFO, "UNIX_TIME_MSEC:       %d msec\n", ctx->unix_time_msec);
      
      /* Sanity check what we got */
      if (!(ctx->obs_duration_sec >= 8 && (ctx->obs_duration_sec % 8 == 0)))
      {
        multilog(log, LOG_ERR, "dada_db_open(): OBS_DURATION is not greater than or equal to 8 or a multiple of 8 seconds.\n");
        return -1;
      }

      if (!(ctx->obs_tiles > 0 && (ctx->obs_tiles % 16 == 0)))
      {
        multilog(log, LOG_ERR, "dada_db_open(): OBS_TILES is not greater than 0 or a multiple of 16 (as required by xGPU).\n");
        return -1;
      }

      if (!(ctx->obs_channel_number > 0 && ctx->obs_channel_number <=24))
      {
        multilog(log, LOG_ERR, "dada_db_open(): OBS_CHANNEL is not between 1 and 24.\n");
        return -1;
      }

      if (!(strlen(ctx->obs_proj_id) == 5))
      {
        multilog(log, LOG_ERR, "dada_db_open(): OBS_PROJ_ID must be 5 characters long.\n");
        return -1;
      }

      if (!(ctx->obs_bandwidth_khz > 0))
      {
        multilog(log, LOG_ERR, "dada_db_open(): OBS_BANDWIDTH is not greater than 0.\n");
        return -1;
      }

      if (!(ctx->obs_pols > 0))
      {
        multilog(log, LOG_ERR, "dada_db_open(): OBS_POLS is not greater than 0.\n");
        return -1;
      }

      if (!(ctx->obs_freq_res_khz >= 1 && ctx->obs_freq_res_khz <= 1280))
      {
        multilog(log, LOG_ERR, "dada_db_open(): OBS_FREQ_RES is not between 1 kHz and 1280 kHz.\n");
        return -1;
      }

      if (!(ctx->obs_int_time_msec >= 200 && ctx->obs_int_time_msec <= 8000))
      {
        multilog(log, LOG_ERR, "dada_db_open(): OBS_INT_TIME is not between than 0.2 and 8 seconds.\n");
        return -1;
      }

      if (!(ctx->transfer_size_bytes > 0))
      {
        multilog(log, LOG_ERR, "dada_db_open(): TRANSFER_SIZE is not greater than 0.\n");
        return -1;
      }

      if (!(ctx->nbit >= 8 && (ctx->nbit % 8 == 0)))
      {
        multilog(log, LOG_ERR, "dada_db_open(): NBIT is not greater than or equal to 8 or a multiple of 8 bits.\n");
        return -1;
      } 

      if (!(ctx->unix_time_msec >= 0 || ctx->unix_time_msec<1000))
      {
        multilog(log, LOG_ERR, "dada_db_open(): UNIX_TIME_MSEC must be between 0 and 999 milliseconds.\n");
        return -1;
      } 

      // Calculate baselines
      ctx->obs_baselines = ((ctx->obs_tiles*(ctx->obs_tiles+1))/2);
      ctx->obs_fine_channels = ctx->obs_bandwidth_khz / ctx->obs_freq_res_khz;

      // Check transfer size read in from header matches what we expect from the other params
      // +1 is for the weights!
      int bytes_per_complex = (ctx->nbit / 8) * 2; // Should be 4 bytes per float (32 bits) x2 for r,i
      uint64_t expected_bytes = ((ctx->obs_pols*ctx->obs_pols)*bytes_per_complex)*ctx->obs_baselines*(ctx->obs_fine_channels+1);
      
      if (expected_bytes != ctx->block_size)
      {
        multilog(log, LOG_ERR, "dada_db_open(): Ring buffer block size (%lu bytes) does not match calculated size from header parameters (%lu bytes).\n", ctx->block_size, expected_bytes);
        return -1;
      }
      
      // Reset the filenumber
      ctx->fits_file_number = 0;

      // These need to be set for psrdadd
      client->transfer_bytes = 0;  
      client->optimal_bytes = 0;

      // we do not want to explicitly transfer the DADA header
      client->header_transfer = 0;
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
      
    /* Make a new filename- oooooooooo_YYYYMMDDhhmmss_gpuboxGG_FF.fits */
    sprintf(ctx->fits_filename, "%s/%ld_%04d%02d%02d%02d%02d%02d_gpubox%02d_%02d.fits", ctx->destination_dir, ctx->obs_id, year, month, day, hour, minute, second, ctx->obs_channel_number, ctx->fits_file_number);
    
    if (create_fits(&ctx->fits_ptr, ctx->fits_filename)) 
    {
      multilog(log, LOG_ERR,"dada_db_open(): Error creating new fits file.\n");
      return -1;
    }
  }
  else
  {
    /* This is a continuation of an existing observation */
    multilog(log, LOG_INFO, "dada_db_open(): continuing %lu...\n", ctx->obs_id);

    /* Update the duration */   
    int new_duration_sec = 0;
    if (ascii_header_get(client->header, "OBS_DURATION", "%i", &new_duration_sec) == -1)
    {
      multilog(log, LOG_ERR, "dada_db_open(): OBS_DURATION not found in header.\n");
      return -1;
    }

    /* has the duration changed? */
    if (new_duration_sec != ctx->obs_duration_sec)
    {
      multilog(log, LOG_INFO, "dada_db_open(): OBS_DURATION has changed from %d sec to %d sec.\n", ctx->obs_duration_sec, new_duration_sec);
    }
    
    /* Update the offset */
    int new_obs_offset_sec = 0;
    if (ascii_header_get(client->header, "OBS_OFFSET", "%i", &new_obs_offset_sec) == -1)
    {
      multilog(log, LOG_ERR, "dada_db_open(): OBS_OFFSET not found in header.\n");
      return -1;
    }

    /* has the offset incremented? */
    if (new_obs_offset_sec == ctx->obs_offset_sec)
    {
      multilog(log, LOG_ERR, "dada_db_open(): OBS_OFFSET is the same as the previous subobservation (%d == %d).\n", ctx->obs_offset_sec, new_obs_offset_sec);
      return -1;
    }
    else if (new_obs_offset_sec < ctx->obs_offset_sec)
    {
      multilog(log, LOG_ERR, "dada_db_open(): OBS_OFFSET is less than the previous observation (%d < %d).\n", ctx->obs_offset_sec, new_obs_offset_sec);
      return -1;
    }
    else if (new_obs_offset_sec - ctx->obs_offset_sec != ctx->obs_subobs_sec)
    {
      multilog(log, LOG_ERR, "dada_db_open(): OBS_OFFSET did not increase by %d seconds (it was: %d).\n", ctx->obs_subobs_sec, new_obs_offset_sec - ctx->obs_offset_sec);
      return -1;
    }
    else
    {
      multilog(log, LOG_INFO, "dada_db_open(): OBS_OFFSET incremented from %d sec to %d sec.\n", ctx->obs_duration_sec, new_duration_sec);
    }
  }
    
  multilog(log, LOG_INFO, "dada_db_open(): completed\n");

  return EXIT_SUCCESS;
}

/**
 * 
 *  @brief This is called at the end of each new 8 second sub-observation.
 *         We need check if the current observation duration has changed (shortened) from what we expected.
 *  @param[in] client A pointer to the dada_client_t object.
 *  @param[in] bytes_written The number of bytes that psrdada has written for this entire 8 second subobservation.
 *  @returns EXIT_SUCCESS on success, or -1 if there was an error.
 */
int dada_dbfits_close(dada_client_t* client, uint64_t bytes_written)
{
  assert (client != 0);
  dada_db_s* ctx = (dada_db_s*) client->context;

  multilog_t *log = (multilog_t *) client->log;
  multilog(log, LOG_INFO, "dada_dbfits_close(bytes_written=%ul): Started.\n", bytes_written);
  
  // Some sanity checks:
  // We should be at a marker which when multiplied by int_time should be a multuple of ctx->obs_secs_per_subobs (8 seconds nominally).
  int current_duration = (int)((float)ctx->obs_marker_number * ((float)ctx->obs_int_time_msec / 1000.0));
  if (current_duration % ctx->obs_subobs_sec != 0)
  {
    multilog(log, LOG_ERR,"dada_dbfits_close(): Error, the dada ringbuffer closed before we got all %d seconds of data!\n", ctx->obs_subobs_sec);
    return -1;
  }

  //
  // TODO: Check with the metabin for info about this observation. Has the duration changed?
  //    
  int new_duration = ctx->obs_duration_sec; //TODO: Fix me! This is a placeholder

  if (ctx->obs_duration_sec != new_duration)
  {
    multilog(log, LOG_INFO,"dada_dbfits_close(): Observation has been cut short. Old duration was %d, new duration is %d.\n", ctx->obs_duration_sec, new_duration);
    ctx->obs_duration_sec = new_duration; // TODO: put new duration from metabin here
  }

  multilog(log, LOG_INFO, "dada_dbfits_close(): Checking duration based on current marker %d vs obs duration %d.\n", current_duration, ctx->obs_duration_sec);

  if (current_duration == ctx->obs_duration_sec)
  {
    // Observation ends NOW! It got cut short, or we naturally are at the end of the observation 
    // Close existing fits file (if we have one)    
    if (ctx->fits_ptr != NULL)
    {
      if (close_fits(&ctx->fits_ptr)) 
      {
        multilog(log, LOG_ERR,"dada_dbfits_close(): Error closing fits file.\n");
        return -1;
      }
    }
  }

  multilog (log, LOG_INFO, "dada_dbfits_close(): completed\n");

  return EXIT_SUCCESS;
}

/**
 * 
 *  @brief This is the function psrdada calls when we have new data to read.
 *  @param[in] client A pointer to the dada_client_t object.
 *  @param[in] buffer The pointer to the data in the ringbuffer we are about to read.
 *  @param[in] bytes The number of bytes that are available to be read from the ringbuffer.
 *  @returns the number of bytes read or -1 if there was an error.
 */
int64_t dada_dbfits_io(dada_client_t *client, void *buffer, uint64_t bytes)
{
  assert (client != 0);
  dada_db_s* ctx = (dada_db_s*) client->context;

  multilog_t * log = (multilog_t *) ctx->log;
  
  uint64_t written  = 0;
  uint64_t to_write = bytes;
  uint64_t wrote    = 0;

  multilog (log, LOG_DEBUG, "dada_dbfits_io(): Processing block %d.\n", ctx->block_number);
   
  while (written < bytes)
  {    
    multilog(log, LOG_INFO, "dada_dbfits_io(): Writing %d into new image HDU; Marker = %d.\n", bytes, ctx->obs_marker_number); 
        
    // Write HDU here! 
    // TODO: Unless we are last block in which case write weights!    
    if (create_fits_imghdu(ctx->fits_ptr, ctx->unix_time, ctx->unix_time_msec, ctx->obs_marker_number, 
                           ctx->obs_baselines, ctx->obs_fine_channels, ctx->obs_pols*ctx->obs_pols,
                           ctx->obs_int_time_msec, (char*)buffer, bytes))    
    {
      // Error!
      multilog(log, LOG_ERR, "dada_dbfits_io(): Error Writing into new image HDU.\n");
      return -1;
    }
    else
    {      
      wrote = to_write;
      written += wrote;
      ctx->obs_marker_number += 1;
    }
  }

  ctx->block_number += 1;
  ctx->bytes_written += written;

  return written;
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
  assert (client != 0);
  dada_db_s* ctx = (dada_db_s*) client->context;

  multilog_t * log = (multilog_t *) ctx->log;

  multilog(log, LOG_INFO, "dada_dbfits_io_block(): Processing block id %llu\n", block_id);

  return dada_dbfits_io(client, buffer, bytes);
}