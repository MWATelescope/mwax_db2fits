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
  if (ascii_header_get(client->header, HEADER_OBS_ID, "%lu", &this_obs_id) == -1)
  {
    multilog(log, LOG_ERR, "dada_db_open(): %s not found in header.\n", HEADER_OBS_ID);
    return -1;
  }

  long this_subobs_id = 0;
  if (ascii_header_get(client->header, HEADER_SUBOBS_ID, "%lu", &this_subobs_id) == -1)
  {
    multilog(log, LOG_ERR, "dada_db_open(): %s not found in header.\n", HEADER_SUBOBS_ID);
    return -1;
  }

  // Sanity check this obs_id
  if (!(this_obs_id > 0))
  {
    multilog(log, LOG_ERR, "dada_db_open(): New %s is not greater than 0.\n", HEADER_OBS_ID);
    return -1;
  }

  // Check this obs_id against our 'in progress' obsid  
  if (ctx->obs_id != this_obs_id || ctx->bytes_written >= FITS_SIZE_CUTOFF_BYTES)
  {
    // We need a new fits file
    if (ctx->obs_id != this_obs_id)
    {
      multilog(log, LOG_INFO, "dada_db_open(): New %s detected. Closing %lu, Starting %lu...\n", HEADER_OBS_ID, ctx->obs_id, this_obs_id);
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

      ctx->obs_id = 0;
      ctx->subobs_id = 0;

      // Read in all of the info from the header into our struct
      if (read_dada_header(client))
      {
        // Error processing in header!
        multilog(log, LOG_ERR,"dada_db_open(): Error processing header.\n");
        return -1;
      }

      /* Set the obsid & sub obsid */
      ctx->obs_id = this_obs_id;
      ctx->subobs_id = this_subobs_id;
                  
      /* Sanity check what we got */
      if (!(ctx->exposure_sec >= 8 && (ctx->exposure_sec % 8 == 0)))
      {
        multilog(log, LOG_ERR, "dada_db_open(): %s is not greater than or equal to 8 or a multiple of 8 seconds.\n", HEADER_EXPOSURE_SEC);
        return -1;
      }

      if (!(ctx->ninputs_xgpu > 0 && (ctx->ninputs_xgpu % 16 == 0)))
      {
        multilog(log, LOG_ERR, "dada_db_open(): %s is not greater than 0 or a multiple of 16 (as required by xGPU).\n", HEADER_NINPUTS_XGPU);
        return -1;
      }

      if (!(ctx->coarse_channel > 0 && ctx->coarse_channel <=24))
      {
        multilog(log, LOG_ERR, "dada_db_open(): %s is not between 1 and 24.\n", HEADER_COARSE_CHANNEL);
        return -1;
      }

      if (!(strlen(ctx->proj_id) == 5))
      {
        multilog(log, LOG_ERR, "dada_db_open(): %s must be 5 characters long.\n", HEADER_PROJ_ID);
        return -1;
      }

      if (!(ctx->bandwidth_hz > 0))
      {
        multilog(log, LOG_ERR, "dada_db_open(): %s is not greater than 0.\n", HEADER_BANDWIDTH_HZ);
        return -1;
      }

      if (!(ctx->npol > 0))
      {
        multilog(log, LOG_ERR, "dada_db_open(): %s is not greater than 0.\n", HEADER_NPOL);
        return -1;
      }

      if (!(ctx->fine_chan_width_hz >= 1 && ctx->fine_chan_width_hz <= 1280000))
      {
        multilog(log, LOG_ERR, "dada_db_open(): %s is not between 1 Hz and 1280000 kHz.\n", HEADER_FINE_CHAN_WIDTH_HZ);
        return -1;
      }

      if (!(ctx->int_time_msec >= 200 && ctx->int_time_msec <= 8000))
      {
        multilog(log, LOG_ERR, "dada_db_open(): %s is not between than 0.2 and 8 seconds.\n", HEADER_INT_TIME_MSEC);
        return -1;
      }

      if (!(ctx->transfer_size > 0))
      {
        multilog(log, LOG_ERR, "dada_db_open(): %s is not greater than 0.\n", HEADER_TRANSFER_SIZE);
        return -1;
      }

      if (!(ctx->nbit >= 8 && (ctx->nbit % 8 == 0)))
      {
        multilog(log, LOG_ERR, "dada_db_open(): %s is not greater than or equal to 8 or a multiple of 8 bits.\n", HEADER_NBIT);
        return -1;
      } 

      if (!(ctx->unix_time_msec >= 0 || ctx->unix_time_msec<1000))
      {
        multilog(log, LOG_ERR, "dada_db_open(): %s must be between 0 and 999 milliseconds.\n", HEADER_UNIXTIME_MSEC);
        return -1;
      } 

      // Calculate baselines
      ctx->nbaselines = (ctx->ninputs_xgpu*(ctx->ninputs_xgpu+2))/8;
      
      // Check transfer size read in from header matches what we expect from the other params
      // +1 is for the weights!
      int bytes_per_complex = (ctx->nbit / 8) * 2; // Should be 4 bytes per float (32 bits) x2 for r,i
      uint64_t expected_bytes = ((ctx->npol*ctx->npol)*bytes_per_complex)*ctx->nbaselines*(ctx->nfine_chan+1);
      
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
    sprintf(ctx->fits_filename, "%s/%ld_%04d%02d%02d%02d%02d%02d_gpubox%02d_%02d.fits", ctx->destination_dir, ctx->obs_id, year, month, day, hour, minute, second, ctx->coarse_channel, ctx->fits_file_number);
    
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
    if (ascii_header_get(client->header, HEADER_EXPOSURE_SEC, "%i", &new_duration_sec) == -1)
    {
      multilog(log, LOG_ERR, "dada_db_open(): %s not found in header.\n", HEADER_EXPOSURE_SEC);
      return -1;
    }

    /* has the duration changed? */
    if (new_duration_sec != ctx->exposure_sec)
    {
      multilog(log, LOG_INFO, "dada_db_open(): %s has changed from %d sec to %d sec.\n", HEADER_EXPOSURE_SEC, ctx->exposure_sec, new_duration_sec);
    }
    
    /* Update the offset */
    int new_obs_offset_sec = 0;
    if (ascii_header_get(client->header, HEADER_OBS_OFFSET, "%i", &new_obs_offset_sec) == -1)
    {
      multilog(log, LOG_ERR, "dada_db_open(): %s not found in header.\n", HEADER_OBS_OFFSET);
      return -1;
    }

    /* has the offset incremented? */
    if (new_obs_offset_sec == ctx->obs_offset)
    {
      multilog(log, LOG_ERR, "dada_db_open(): %s is the same as the previous subobservation (%d == %d).\n", HEADER_OBS_OFFSET, ctx->obs_offset, new_obs_offset_sec);
      return -1;
    }
    else if (new_obs_offset_sec < ctx->obs_offset)
    {
      multilog(log, LOG_ERR, "dada_db_open(): %s is less than the previous observation (%d < %d).\n", HEADER_OBS_OFFSET, ctx->obs_offset, new_obs_offset_sec);
      return -1;
    }
    else if (new_obs_offset_sec - ctx->obs_offset != ctx->secs_per_subobs)
    {
      multilog(log, LOG_ERR, "dada_db_open(): %s did not increase by %d seconds (it was: %d).\n", HEADER_OBS_OFFSET, ctx->secs_per_subobs, new_obs_offset_sec - ctx->obs_offset);
      return -1;
    }
    else
    {
      multilog(log, LOG_INFO, "dada_db_open(): %s incremented from %d sec to %d sec.\n", HEADER_OBS_OFFSET, ctx->exposure_sec, new_duration_sec);
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
  int current_duration = (int)((float)ctx->obs_marker_number * ((float)ctx->int_time_msec / 1000.0));
  if (current_duration % ctx->secs_per_subobs != 0)
  {
    multilog(log, LOG_ERR,"dada_dbfits_close(): Error, the dada ringbuffer closed before we got all %d seconds of data!\n", ctx->secs_per_subobs);
    return -1;
  }

  //
  // TODO: Check with the metabin for info about this observation. Has the duration changed?
  //    
  int new_duration = ctx->exposure_sec; //TODO: Fix me! This is a placeholder

  if (ctx->exposure_sec != new_duration)
  {
    multilog(log, LOG_INFO,"dada_dbfits_close(): Observation has been cut short. Old duration was %d, new duration is %d.\n", ctx->exposure_sec, new_duration);
    ctx->exposure_sec = new_duration; // TODO: put new duration from metabin here
  }

  multilog(log, LOG_INFO, "dada_dbfits_close(): Checking duration based on current marker %d vs obs duration %d.\n", current_duration, ctx->exposure_sec);

  if (current_duration == ctx->exposure_sec)
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
                           ctx->nbaselines, ctx->nfine_chan, ctx->npol*ctx->npol,
                           ctx->int_time_msec, (char*)buffer, bytes))    
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

int read_dada_header(dada_client_t *client)
{
  // Reset and read everything except for obs_id and subobs_id
  assert(client != 0);
  dada_db_s* ctx = (dada_db_s*) client->context;

  assert(ctx->log != 0);
  multilog_t * log = (multilog_t *) ctx->log;

  ctx->populated = 0;  
  strncpy(ctx->command, "", MWAX_COMMAND_LEN);
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
  ctx->secs_per_subobs = 0;
  ctx->unix_time = 0;
  ctx->unix_time_msec = 0;
  ctx->fine_chan_width_hz = 0;
  ctx->nfine_chan = 0;
  ctx->bandwidth_hz = 0;

  ctx->nbaselines = 0;                           
  ctx->obs_marker_number = 0;
  
  if (ascii_header_get(client->header, HEADER_POPULATED, "%i", &ctx->populated) == -1)
  {
    multilog(log, LOG_ERR, "dada_db_open(): %s not found in header.\n", HEADER_POPULATED);
    return -1;
  }

  if (ascii_header_get(client->header, HEADER_COMMAND, "%s", &ctx->command) == -1)
  {
    multilog(log, LOG_ERR, "dada_db_open(): %s not found in header.\n", HEADER_COMMAND);
    return -1;
  }

  if (ascii_header_get(client->header, HEADER_UTC_START, "%s", &ctx->utc_start) == -1)
  {
    multilog(log, LOG_ERR, "dada_db_open(): %s not found in header.\n", HEADER_UTC_START);
    return -1;
  }

  if (ascii_header_get(client->header, HEADER_OBS_OFFSET, "%i", &ctx->obs_offset) == -1)
  {
    multilog(log, LOG_ERR, "dada_db_open(): %s not found in header.\n", HEADER_OBS_OFFSET);
    return -1;
  }

  if (ascii_header_get(client->header, HEADER_NBIT, "%i", &ctx->nbit) == -1)
  {
    multilog(log, LOG_ERR, "dada_db_open(): %s not found in header.\n", HEADER_NBIT);
    return -1;
  }

  if (ascii_header_get(client->header, HEADER_NPOL, "%i", &ctx->npol) == -1)
  {
    multilog(log, LOG_ERR, "dada_db_open(): %s not found in header.\n", HEADER_NPOL);
    return -1;
  }

  if (ascii_header_get(client->header, HEADER_NINPUTS_XGPU, "%i", &ctx->ninputs_xgpu) == -1)
  {
    multilog(log, LOG_ERR, "dada_db_open(): %s not found in header.\n", HEADER_NINPUTS_XGPU);
    return -1;
  }

  if (ascii_header_get(client->header, HEADER_INT_TIME_MSEC, "%i", &ctx->int_time_msec) == -1)
  {
    multilog(log, LOG_ERR, "dada_db_open(): %s not found in header.\n", HEADER_INT_TIME_MSEC);
    return -1;
  }

  if (ascii_header_get(client->header, HEADER_TRANSFER_SIZE, "%lu", &ctx->transfer_size) == -1)
  {
    multilog(log, LOG_ERR, "dada_db_open(): %s not found in header.\n", HEADER_TRANSFER_SIZE);
    return -1;
  }

  if (ascii_header_get(client->header, HEADER_PROJ_ID, "%s", &ctx->proj_id) == -1)
  {
    multilog(log, LOG_ERR, "dada_db_open(): %s not found in header.\n", HEADER_PROJ_ID);
    return -1;
  }

  if (ascii_header_get(client->header, HEADER_EXPOSURE_SEC, "%i", &ctx->exposure_sec) == -1)
  {
    multilog(log, LOG_ERR, "dada_db_open(): %s not found in header.\n", HEADER_EXPOSURE_SEC);
    return -1;
  }

  if (ascii_header_get(client->header, HEADER_COARSE_CHANNEL, "%i", &ctx->coarse_channel) == -1)
  {
    multilog(log, LOG_ERR, "dada_db_open(): %s not found in header.\n", HEADER_COARSE_CHANNEL);
    return -1;
  }

  if (ascii_header_get(client->header, HEADER_SECS_PER_SUBOBS, "%i", &ctx->secs_per_subobs) == -1)
  {
    multilog(log, LOG_ERR, "dada_db_open(): %s not found in header.\n", HEADER_SECS_PER_SUBOBS);
    return -1;
  }

  if (ascii_header_get(client->header, HEADER_UNIXTIME, "%lu", &ctx->unix_time) == -1)
  {
    multilog(log, LOG_ERR, "dada_db_open(): %s not found in header.\n", HEADER_UNIXTIME);
    return -1;
  }

  if (ascii_header_get(client->header, HEADER_UNIXTIME_MSEC, "%i", &ctx->unix_time_msec) == -1)
  {
    multilog(log, LOG_ERR, "dada_db_open(): %s not found in header.\n", HEADER_UNIXTIME_MSEC);
    return -1;
  }
    
  if (ascii_header_get(client->header, HEADER_FINE_CHAN_WIDTH_HZ, "%i", &ctx->fine_chan_width_hz) == -1)
  {
    multilog(log, LOG_ERR, "dada_db_open(): %s not found in header.\n", HEADER_FINE_CHAN_WIDTH_HZ);
    return -1;
  }

  if (ascii_header_get(client->header, HEADER_NFINE_CHAN, "%i", &ctx->nfine_chan) == -1)
  {
    multilog(log, LOG_ERR, "dada_db_open(): %s not found in header.\n", HEADER_NFINE_CHAN);
    return -1;
  }  

  if (ascii_header_get(client->header, HEADER_BANDWIDTH_HZ, "%i", &ctx->bandwidth_hz) == -1)
  {
    multilog(log, LOG_ERR, "dada_db_open(): %s not found in header.\n", HEADER_BANDWIDTH_HZ);
    return -1;
  }  
  

  // Output what we found in the header
  multilog(log, LOG_INFO, "Populated?:               %s\n", (ctx->populated==1?"yes":"no"));
  multilog(log, LOG_INFO, "Obs Id:                   %lu\n", ctx->obs_id);
  multilog(log, LOG_INFO, "Subobs Id:                %lu\n", ctx->subobs_id);
  multilog(log, LOG_INFO, "Command:                  %s\n", ctx->command);
  multilog(log, LOG_INFO, "Start time (UTC):         %s\n", ctx->utc_start);
  multilog(log, LOG_INFO, "Offset:                   %d sec\n", ctx->obs_offset);
  multilog(log, LOG_INFO, "Bits per real/imag:       %d\n", ctx->nbit);
  multilog(log, LOG_INFO, "Polarisations:            %d\n", ctx->npol);
  multilog(log, LOG_INFO, "Tiles:                    %d\n", ctx->ninputs_xgpu / 2);
  multilog(log, LOG_INFO, "Correlator int time:      %d msec\n", ctx->int_time_msec / 1000);        
  multilog(log, LOG_INFO, "Size of observation:      %lu bytes\n", ctx->transfer_size);
  multilog(log, LOG_INFO, "Project Id:               %s\n", ctx->proj_id);  
  multilog(log, LOG_INFO, "Duration:                 %d sec\n", ctx->exposure_sec);
  multilog(log, LOG_INFO, "Coarse channel no.:       %d\n", ctx->coarse_channel);
  multilog(log, LOG_INFO, "Duration of subobs:       %d sec\n", ctx->secs_per_subobs);
  multilog(log, LOG_INFO, "UNIX time of subobs:      %lu\n", ctx->unix_time);
  multilog(log, LOG_INFO, "UNIX milliseconds:        %d msec\n", ctx->unix_time_msec);
  multilog(log, LOG_INFO, "Correlator freq res:      %d kHz\n", ctx->fine_chan_width_hz / 1000);
  multilog(log, LOG_INFO, "No fine chans per coarse: %d\n", ctx->nfine_chan);
  multilog(log, LOG_INFO, "Coarse channel width:     %d kHz\n", ctx->bandwidth_hz / 1000);

  return EXIT_SUCCESS;
}