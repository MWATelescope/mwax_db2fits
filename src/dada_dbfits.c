/*
 * dada_dbfits.c
 *
 *  Created on: 23-May-2018
 *      Author: Greg Sleap
 */
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
int dada_dbfits_open (dada_client_t* client)
{
  assert (client != 0);
  dada_db_t* ctx = (dada_db_t*) client->context;

  multilog_t *log = (multilog_t *) client->log;

  multilog (log, LOG_INFO, "dada_db_open()\n");

  // initialise
  ctx->block_open = 0;
  ctx->bytes_read = 0;
  ctx->bytes_written = 0;
  ctx->curr_block = 0;
  
  multilog(log, LOG_INFO, "dada_db_open: extracting params from header\n");

  // get the transfer size (if it is set)  
  //ascii_header_get(client->header, "TRANSFER_SIZE", "%"PRIi64, &transfer_size);
  
  client->transfer_bytes = 0;  // CHECK: transfer_size may be zero
  client->optimal_bytes = 64*1024*1024;

  // we do not want to explicitly transfer the DADA header
  //client->header_transfer = 0;
  
  multilog (log, LOG_INFO, "dada_db_open: completed\n");

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
  multilog (log, LOG_INFO, "dada_db_close()\n");
  
  multilog (log, LOG_INFO, "dada_db_close: completed\n");

  return 0;
}

/******************************************************************************/
/* used for transferring the header, and uses pageable memory                 */
/******************************************************************************/
int64_t dada_dbfits_io (dada_client_t *client, void *buffer, uint64_t bytes)
{
  assert (client != 0);
  dada_db_t* ctx = (dada_db_t*) client->context;

  multilog_t * log = (multilog_t *) ctx->log;
  
  uint64_t written  = 0;
  uint64_t to_write = 512;
  uint64_t wrote    = 0;

  multilog (log, LOG_INFO, "Block!\n");

  while (written < bytes)
  {    
    wrote = to_write;

    written += wrote;

    multilog (log, LOG_INFO, "...getting data! input %d, written %d\n", bytes, written); //buffer + written
  }

  return (int64_t)written;
}

int64_t dada_dbfits_io_block(dada_client_t *client, void *buffer, uint64_t bytes, uint64_t block_id)
{
  return dada_dbfits_io(client, buffer, bytes);
}