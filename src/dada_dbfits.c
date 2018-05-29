/*
 * dada_dbfits.c
 *
 *  Created on: 23-May-2018
 *      Author: Greg Sleap
 */
#include "utils.h"
#include "dada_dbfits.h"

int dada_dbfits_init(dada_db_t* ctx, dada_hdu_t * in_hdu)
{
  multilog_t * log = ctx->log;

  // check the header block sizes for input
  ctx->header_size = ipcbuf_get_bufsz (in_hdu->header_block);
  
  // input blocks 
  ctx->block_size = ipcbuf_get_bufsz ((ipcbuf_t *) in_hdu->data_block);  
      
  multilog (log, LOG_INFO, "dada_db_init: completed\n");

  return 0;
}

/******************************************************************************/
/* open dada_db client                                                        */
/******************************************************************************/
int dada_dbfits_open(dada_client_t* client)
{
  assert(client != 0);
  dada_db_t* ctx = (dada_db_t*) client->context;

  multilog_t *log = (multilog_t *) client->log;

  multilog(log, LOG_INFO, "dada_db_open()\n");

  // initialise
  ctx->block_open = 0;
  ctx->bytes_read = 0;
  ctx->bytes_written = 0;
  ctx->curr_block = 0;
  ctx->block_number= 0;

  // fits info  
  strcpy(ctx->fits_filename, "");

  // obs info
  ctx->obs_id = 0;
  ctx->obs_duration = 0;
  ctx->obs_channel = 0;
  strcpy(ctx->obs_proj_id, "");
  ctx->obs_tiles = 0;
  ctx->obs_bandwidth = 0;
  ctx->obs_pols = 0;
  ctx->obs_freq_res = 0;
  ctx->obs_int_time = 0;
  ctx->transfer_size = 0;
    
  multilog(log, LOG_INFO, "dada_db_open: extracting params from header\n");

  // get the transfer size (if it is set)  
  if (ascii_header_get(client->header, "OBS_ID", "%lu", &ctx->obs_id) == -1)
  {
    multilog(log, LOG_ERR, "OBS_ID not found in header.\n");
    return -1;
  }

  if (ascii_header_get(client->header, "OBS_DURATION", "%i", &ctx->obs_duration) == -1)
  {
    multilog(log, LOG_ERR, "OBS_DURATION not found in header.\n");
    return -1;
  }

  if (ascii_header_get(client->header, "OBS_CHANNEL", "%i", &ctx->obs_channel) == -1)
  {
    multilog(log, LOG_ERR, "OBS_CHANNEL not found in header.\n");
    return -1;
  }

  if (ascii_header_get(client->header, "OBS_PROJ_ID", "%s", &ctx->obs_proj_id) == -1)
  {
    multilog(log, LOG_ERR, "OBS_PROJ_ID not found in header.\n");
    return -1;
  }

  if (ascii_header_get(client->header, "OBS_BANDWIDTH", "%i", &ctx->obs_bandwidth) == -1)
  {
    multilog(log, LOG_ERR, "OBS_BANDWIDTH not found in header.\n");
    return -1;
  }

  if (ascii_header_get(client->header, "OBS_TILES", "%i", &ctx->obs_tiles) == -1)
  {
    multilog(log, LOG_ERR, "OBS_TILES not found in header.\n");
    return -1;
  }

  if (ascii_header_get(client->header, "OBS_POLS", "%i", &ctx->obs_pols) == -1)
  {
    multilog(log, LOG_ERR, "OBS_POLS not found in header.\n");
    return -1;
  }

  if (ascii_header_get(client->header, "OBS_FREQ_RES", "%i", &ctx->obs_freq_res) == -1)
  {
    multilog(log, LOG_ERR, "OBS_FREQ_RES not found in header.\n");
    return -1;
  }

  if (ascii_header_get(client->header, "OBS_INT_TIME", "%f", &ctx->obs_int_time) == -1)
  {
    multilog(log, LOG_ERR, "OBS_INT_TIME not found in header.\n");
    return -1;
  }

  if (ascii_header_get(client->header, "TRANSFER_SIZE", "%lu", &ctx->transfer_size) == -1)
  {
    multilog(log, LOG_ERR, "TRANSFER_SIZE not found in header.\n");
    return -1;
  }

  // Output what we found in the header
  multilog(log, LOG_INFO, "OBS_ID:        %lu\n", ctx->obs_id);
  multilog(log, LOG_INFO, "OBS_DURATION:  %i sec\n", ctx->obs_duration);
  multilog(log, LOG_INFO, "OBS_CHANNEL:   %i\n", ctx->obs_channel);
  multilog(log, LOG_INFO, "OBS_PROJ_ID:   %s\n", ctx->obs_proj_id);
  multilog(log, LOG_INFO, "OBS_TILES:     %i\n", ctx->obs_tiles);
  multilog(log, LOG_INFO, "OBS_BANDWIDTH: %i kHz\n", ctx->obs_bandwidth);
  multilog(log, LOG_INFO, "OBS_POLS:      %i\n", ctx->obs_pols);
  multilog(log, LOG_INFO, "OBS_FREQ_RES:  %i kHz\n", ctx->obs_freq_res);
  multilog(log, LOG_INFO, "OBS_INT_TIME:  %f sec\n", ctx->obs_int_time);
  multilog(log, LOG_INFO, "TRANSFER_SIZE: %lu bytes\n", ctx->transfer_size);

  if (!(ctx->obs_id > 0))
  {
    multilog(log, LOG_ERR, "OBS_ID is not greater than 0.\n");
    return -1;
  }

  if (!(ctx->obs_duration >= 8 && (ctx->obs_duration % 8 == 0)))
  {
    multilog(log, LOG_ERR, "OBS_DURATION is not greater than or equal to 8 or a multiple of 8 seconds.\n");
    return -1;
  }

  if (!(ctx->obs_tiles > 0 && (ctx->obs_tiles % 16 == 0)))
  {
    multilog(log, LOG_ERR, "OBS_TILES is not greater than 0 or a multiple of 16 (as required by xGPU).\n");
    return -1;
  }

  if (!(ctx->obs_channel > 0 && ctx->obs_channel <=24))
  {
    multilog(log, LOG_ERR, "OBS_CHANNEL is not between 1 and 24.\n");
    return -1;
  }

  if (!(strlen(ctx->obs_proj_id) == 5))
  {
    multilog(log, LOG_ERR, "OBS_PROJ_ID must be 5 characters long.\n");
    return -1;
  }

  if (!(ctx->obs_bandwidth > 0))
  {
    multilog(log, LOG_ERR, "OBS_BANDWIDTH is not greater than 0.\n");
    return -1;
  }

  if (!(ctx->obs_pols > 0))
  {
    multilog(log, LOG_ERR, "OBS_POLS is not greater than 0.\n");
    return -1;
  }

  if (!(ctx->obs_freq_res >= 0.125 && ctx->obs_freq_res <= 1280))
  {
    multilog(log, LOG_ERR, "OBS_FREQ_RES is not between 0.125 kHz and 1280 kHz.\n");
    return -1;
  }

  if (!(ctx->obs_int_time >= 0.2 && ctx->obs_int_time <= 8))
  {
    multilog(log, LOG_ERR, "OBS_INT_TIME is not between than 0.2 and 8 seconds.\n");
    return -1;
  }

  if (!(ctx->transfer_size > 0))
  {
    multilog(log, LOG_ERR, "TRANSFER_SIZE is not greater than 0.\n");
    return -1;
  }

  // Check transfer size read in from header matches what we expect from the other params
  // +1 is for the weights!
  long expected_bytes = (ctx->obs_pols*8)*((ctx->obs_tiles*(ctx->obs_tiles+1))/2)*((ctx->obs_bandwidth/ctx->obs_freq_res)+1);
  
  if (expected_bytes != ctx->transfer_size)
  {
    multilog(log, LOG_ERR, "TRANSFER_SIZE in header (%lu bytes) does not match calculated size from header parameters (%lu bytes).\n", ctx->transfer_size, expected_bytes);
    return -1;
  }
      
  client->transfer_bytes = 0;  
  client->optimal_bytes = 64*1024*1024;

  // we do not want to explicitly transfer the DADA header
  client->header_transfer = 0;
  
  // Create fits file for output  
  // Get timestamp  
  char timestring[15];
  get_time_string_for_fits(timestring);  
  int file_number = 0;  
    
  // Make a new filename- oooooooooo_YYYYMMDDhhmmss_gpuboxGG_FF.fits
  sprintf(ctx->fits_filename, "%ld_%s_gpubox%02d_%02d.fits", ctx->obs_id, timestring, ctx->obs_channel, file_number);
      
  if (create_fits(&ctx->fits_ptr, ctx->fits_filename)) {
    multilog(log, LOG_ERR,"Error creating new fits file.\n");
    exit(EXIT_FAILURE);
  }

  multilog(log, LOG_INFO, "dada_db_open: completed\n");

  return 0;
}

/******************************************************************************/
/* close dada_db client                                                        */
/******************************************************************************/
int dada_dbfits_close(dada_client_t* client, uint64_t bytes_written)
{
  assert (client != 0);
  dada_db_t* ctx = (dada_db_t*) client->context;

  multilog_t *log = (multilog_t *) client->log;
  multilog(log, LOG_INFO, "dada_db_close()\n");
  
  // Close the fits file we created
  if (ctx->fits_ptr != NULL)
  {
    if (close_fits(ctx->fits_ptr)) {
      multilog(log, LOG_ERR,"Error closing fits file.\n");
      return -1;
    }
  }

  multilog (log, LOG_INFO, "dada_db_close: completed\n");

  return 0;
}

int64_t dada_dbfits_io (dada_client_t *client, void *buffer, uint64_t bytes)
{
  assert (client != 0);
  dada_db_t* ctx = (dada_db_t*) client->context;

  multilog_t * log = (multilog_t *) ctx->log;
  
  uint64_t written  = 0;
  uint64_t to_write = bytes;
  uint64_t wrote    = 0;

  multilog (log, LOG_INFO, "Block %i!\n", ctx->block_number);

  while (written < bytes)
  {    
    // Write HDU here! Unless we are last block in which case write weights!
    
    wrote = to_write;
    written += wrote;

    multilog (log, LOG_INFO, "...getting data! input %d, written %d\n", bytes, written); //buffer + written
  }

  ctx->block_number += 1;

  return (int64_t)written;
}

int64_t dada_dbfits_io_block(dada_client_t *client, void *buffer, uint64_t bytes, uint64_t block_id)
{
  return dada_dbfits_io(client, buffer, bytes);
}