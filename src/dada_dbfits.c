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
 *  @returns EXIT_SUCCESS on success, or EXIT_FAILURE if there was an error. 
 */
int dada_dbfits_open(dada_client_t* client)
{
  assert(client != 0);
  dada_db_s* ctx = (dada_db_s*) client->context;

  multilog_t *log = (multilog_t *) client->log;  

  multilog(log, LOG_INFO, "dada_db_open(): extracting params from dada header\n");

  // get the obs_id
  long this_obs_id = 0;
  if (ascii_header_get(client->header, "OBS_ID", "%lu", &this_obs_id) == -1)
  {
    multilog(log, LOG_ERR, "OBS_ID not found in header.\n");
    return EXIT_FAILURE;
  }

  // Sanity check this obs_id
  if (!(this_obs_id > 0))
  {
    multilog(log, LOG_ERR, "New OBS_ID is not greater than 0.\n");
    return EXIT_FAILURE;
  }

  // Check this obs_id against our 'in progress' obsid
  if (ctx->obs_id != this_obs_id)
  {
    // This is a NEW observation!
    multilog(log, LOG_INFO, "New OBS_ID detected. Closing %lu, Starting %lu...\n", ctx->obs_id, this_obs_id);

    // Close existing fits file (if we have one)    
    if (ctx->fits_ptr != NULL)
    {
      if (close_fits(&ctx->fits_ptr)) 
      {
        multilog(log, LOG_ERR,"Error closing fits file.\n");
        return EXIT_FAILURE;
      }
    }

    // initialise our structure
    ctx->block_open = 0;
    ctx->bytes_read = 0;
    ctx->bytes_written = 0;
    ctx->curr_block = 0;
    ctx->block_number= 0;

    // fits info  
    strcpy(ctx->fits_filename, "");

    // obs info
    ctx->obs_id = this_obs_id;
    ctx->obs_duration = 0;
    ctx->obs_channel = 0;
    strcpy(ctx->obs_proj_id, "");
    ctx->obs_tiles = 0;
    ctx->obs_bandwidth = 0;
    ctx->obs_pols = 0;
    ctx->obs_freq_res = 0;
    ctx->obs_int_time = 0;
    ctx->obs_secs_per_subobs = 0;
    ctx->transfer_size = 0;
    ctx->obs_marker = 0;

    if (ascii_header_get(client->header, "OBS_DURATION", "%i", &ctx->obs_duration) == -1)
    {
      multilog(log, LOG_ERR, "OBS_DURATION not found in header.\n");
      return EXIT_FAILURE;
    }

    if (ascii_header_get(client->header, "OBS_CHANNEL", "%i", &ctx->obs_channel) == -1)
    {
      multilog(log, LOG_ERR, "OBS_CHANNEL not found in header.\n");
      return EXIT_FAILURE;
    }

    if (ascii_header_get(client->header, "OBS_PROJ_ID", "%s", &ctx->obs_proj_id) == -1)
    {
      multilog(log, LOG_ERR, "OBS_PROJ_ID not found in header.\n");
      return EXIT_FAILURE;
    }

    if (ascii_header_get(client->header, "OBS_BANDWIDTH", "%i", &ctx->obs_bandwidth) == -1)
    {
      multilog(log, LOG_ERR, "OBS_BANDWIDTH not found in header.\n");
      return EXIT_FAILURE;
    }

    if (ascii_header_get(client->header, "OBS_TILES", "%i", &ctx->obs_tiles) == -1)
    {
      multilog(log, LOG_ERR, "OBS_TILES not found in header.\n");
      return EXIT_FAILURE;
    }

    if (ascii_header_get(client->header, "OBS_POLS", "%i", &ctx->obs_pols) == -1)
    {
      multilog(log, LOG_ERR, "OBS_POLS not found in header.\n");
      return EXIT_FAILURE;
    }

    if (ascii_header_get(client->header, "OBS_FREQ_RES", "%i", &ctx->obs_freq_res) == -1)
    {
      multilog(log, LOG_ERR, "OBS_FREQ_RES not found in header.\n");
      return EXIT_FAILURE;
    }

    if (ascii_header_get(client->header, "OBS_INT_TIME", "%f", &ctx->obs_int_time) == -1)
    {
      multilog(log, LOG_ERR, "OBS_INT_TIME not found in header.\n");
      return EXIT_FAILURE;
    }

    if (ascii_header_get(client->header, "OBS_SECS_PER_SUBOBS", "%i", &ctx->obs_secs_per_subobs) == -1)
    {
      multilog(log, LOG_ERR, "OBS_SECS_PER_SUBOBS not found in header.\n");
      return EXIT_FAILURE;
    }

    if (ascii_header_get(client->header, "TRANSFER_SIZE", "%lu", &ctx->transfer_size) == -1)
    {
      multilog(log, LOG_ERR, "TRANSFER_SIZE not found in header.\n");
      return EXIT_FAILURE;
    }

    // Output what we found in the header
    multilog(log, LOG_INFO, "OBS_ID:               %lu\n", ctx->obs_id);
    multilog(log, LOG_INFO, "OBS_DURATION:         %d sec\n", ctx->obs_duration);
    multilog(log, LOG_INFO, "OBS_CHANNEL:          %d\n", ctx->obs_channel);
    multilog(log, LOG_INFO, "OBS_PROJ_ID:          %s\n", ctx->obs_proj_id);
    multilog(log, LOG_INFO, "OBS_TILES:            %d\n", ctx->obs_tiles);
    multilog(log, LOG_INFO, "OBS_BANDWIDTH:        %d kHz\n", ctx->obs_bandwidth);
    multilog(log, LOG_INFO, "OBS_POLS:             %d\n", ctx->obs_pols);
    multilog(log, LOG_INFO, "OBS_FREQ_RES:         %d kHz\n", ctx->obs_freq_res);
    multilog(log, LOG_INFO, "OBS_INT_TIME:         %f sec\n", ctx->obs_int_time);
    multilog(log, LOG_INFO, "OBS_SECS_PER_SUBOBS:  %d sec\n", ctx->obs_secs_per_subobs);
    multilog(log, LOG_INFO, "TRANSFER_SIZE:        %lu bytes\n", ctx->transfer_size);
    
    if (!(ctx->obs_duration >= 8 && (ctx->obs_duration % 8 == 0)))
    {
      multilog(log, LOG_ERR, "OBS_DURATION is not greater than or equal to 8 or a multiple of 8 seconds.\n");
      return EXIT_FAILURE;
    }

    if (!(ctx->obs_tiles > 0 && (ctx->obs_tiles % 16 == 0)))
    {
      multilog(log, LOG_ERR, "OBS_TILES is not greater than 0 or a multiple of 16 (as required by xGPU).\n");
      return EXIT_FAILURE;
    }

    if (!(ctx->obs_channel > 0 && ctx->obs_channel <=24))
    {
      multilog(log, LOG_ERR, "OBS_CHANNEL is not between 1 and 24.\n");
      return EXIT_FAILURE;
    }

    if (!(strlen(ctx->obs_proj_id) == 5))
    {
      multilog(log, LOG_ERR, "OBS_PROJ_ID must be 5 characters long.\n");
      return EXIT_FAILURE;
    }

    if (!(ctx->obs_bandwidth > 0))
    {
      multilog(log, LOG_ERR, "OBS_BANDWIDTH is not greater than 0.\n");
      return EXIT_FAILURE;
    }

    if (!(ctx->obs_pols > 0))
    {
      multilog(log, LOG_ERR, "OBS_POLS is not greater than 0.\n");
      return EXIT_FAILURE;
    }

    if (!(ctx->obs_freq_res >= 0.125 && ctx->obs_freq_res <= 1280))
    {
      multilog(log, LOG_ERR, "OBS_FREQ_RES is not between 0.125 kHz and 1280 kHz.\n");
      return EXIT_FAILURE;
    }

    if (!(ctx->obs_int_time >= 0.2 && ctx->obs_int_time <= 8))
    {
      multilog(log, LOG_ERR, "OBS_INT_TIME is not between than 0.2 and 8 seconds.\n");
      return EXIT_FAILURE;
    }

    if (!(ctx->transfer_size > 0))
    {
      multilog(log, LOG_ERR, "TRANSFER_SIZE is not greater than 0.\n");
      return EXIT_FAILURE;
    }

    // Calculate baselines
    ctx->obs_baselines = ((ctx->obs_tiles*(ctx->obs_tiles+1))/2);
    ctx->obs_fine_channels = ctx->obs_bandwidth/ctx->obs_freq_res;

    // Check transfer size read in from header matches what we expect from the other params
    // +1 is for the weights!
    long expected_bytes = ((ctx->obs_pols*ctx->obs_pols)*8)*ctx->obs_baselines*ctx->obs_fine_channels;
    
    if (expected_bytes != ctx->transfer_size)
    {
      multilog(log, LOG_ERR, "TRANSFER_SIZE in header (%lu bytes) does not match calculated size from header parameters (%lu bytes).\n", ctx->transfer_size, expected_bytes);
      return EXIT_FAILURE;
    }
        
    client->transfer_bytes = 0;  
    client->optimal_bytes = 64*1024*1024;

    // we do not want to explicitly transfer the DADA header
    client->header_transfer = 0;
    
    // Create fits file for output  
    // Get timestamp  
    char timestring[15] = "";
    // Override timestring
    //get_time_string_for_fits(timestring);      
    int file_number = 0;  
      
    // Make a new filename- oooooooooo_YYYYMMDDhhmmss_gpuboxGG_FF.fits
    sprintf(ctx->fits_filename, "%s/%ld_%s_gpubox%02d_%02d.fits", ctx->destination_dir, ctx->obs_id, timestring, ctx->obs_channel, file_number);
    
    if (create_fits(&ctx->fits_ptr, ctx->fits_filename)) 
    {
      multilog(log, LOG_ERR,"Error creating new fits file.\n");
      exit(EXIT_FAILURE);
    }
  }
  else
  {
    // This is a continuation of an existing observation
  }
  
  multilog(log, LOG_INFO, "dada_db_open: completed\n");

  return EXIT_SUCCESS;
}

/**
 * 
 *  @brief This is called at the end of each new 8 second sub-observation.
 *         We need check if the current observation duration has changed (shortened) from what we expected.
 *  @param[in] client A pointer to the dada_client_t object.
 *  @param[in] bytes_written The number of bytes that psrdada has written for this entire 8 second subobservation.
 *  @returns EXIT_SUCCESS on success, or EXIT_FAILURE if there was an error.
 */
int dada_dbfits_close(dada_client_t* client, uint64_t bytes_written)
{
  assert (client != 0);
  dada_db_s* ctx = (dada_db_s*) client->context;

  multilog_t *log = (multilog_t *) client->log;
  multilog(log, LOG_INFO, "dada_dbfits_close(): Started.\n");
  
  // Some sanity checks:
  // We should be at a marker which when multiplied by int_time should be a multuple of ctx->obs_secs_per_subobs (8 seconds nominally).
  int current_duration = (int)((float)ctx->obs_marker * ctx->obs_int_time);
  if (current_duration % ctx->obs_secs_per_subobs != 0)
  {
    multilog(log, LOG_ERR,"dada_dbfits_close(): Error, the dada ringbuffer closed before we got all %d seconds of data!\n", ctx->obs_secs_per_subobs);
    return EXIT_FAILURE;
  }

  //
  // TODO: Check with the metabin for info about this observation. Has the duration changed?
  //    
  int new_duration = ctx->obs_duration;

  if (ctx->obs_duration != new_duration)
  {
    multilog(log, LOG_INFO,"dada_dbfits_close(): Observation has been cut short. Old duration was %d, new duration is %d.\n", ctx->obs_duration, new_duration);
    ctx->obs_duration = new_duration; // TODO: put new duration from metabin here
  }

  multilog(log, LOG_INFO, "dada_dbfits_close(): Checking duration based on current marker %d vs obs duration %d.\n", current_duration, ctx->obs_duration);

  if (current_duration == ctx->obs_duration)
  {
    // Observation ends NOW! It got cut short, or we naturally are at the end of the observation 
    // Close existing fits file (if we have one)    
    if (ctx->fits_ptr != NULL)
    {
      if (close_fits(&ctx->fits_ptr)) 
      {
        multilog(log, LOG_ERR,"dada_dbfits_close(): Error closing fits file.\n");
        return EXIT_FAILURE;
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

  time_t unix_time = 0;
  int unix_milliseconds_time = 0;
  
  while (written < bytes)
  {    
    multilog(log, LOG_INFO, "dada_dbfits_io(): Writing %d into new image HDU; Marker = %d.\n", bytes, ctx->obs_marker); 
        
    // Write HDU here! 
    // TODO: Unless we are last block in which case write weights!    
    if (create_fits_imghdu(ctx->fits_ptr, unix_time, unix_milliseconds_time, ctx->obs_marker, 
                           ctx->obs_baselines, ctx->obs_fine_channels, ctx->obs_pols*ctx->obs_pols,
                           ctx->obs_int_time, (char*)buffer, bytes))    
    {
      // Error!
      multilog(log, LOG_ERR, "dada_dbfits_io(): Error Writing into new image HDU.\n");
      return -1;
    }
    else
    {      
      wrote = to_write;
      written += wrote;
      ctx->obs_marker += 1;
    }
  }

  ctx->block_number += 1;

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