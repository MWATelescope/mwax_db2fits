/*
 * main.c
 *
 *  Created on: 15-May-2018
 *      Author: Greg Sleap
 */
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>

#include "main.h"
#include "args.h"
#include "fitsio.h"
#include "fitswriter.h"
#include "log.h"
#include "utils.h"
#include "dada_dbfits.h"

int quit = 0;
int LOG_LEVEL=XC_LOG_DEBUG;
globalArgs_t globalArgs;

int main(int argc, char* argv[])
{	  
  log_info("Starting up mwa_xc_datacapture...");
  
  if (process_args(argc, argv))
  {
    exit(EXIT_FAILURE);
  }
  
  // Read from dada shared memory
  dada_db_t ctx;

  // DADA Header plus Data Units
  dada_hdu_t* in_hdu = 0;

  // DADA Primary Read Client
  dada_client_t* client = 0;

  // DADA Logger
  multilog_t* log = 0;

  // DADA keys for input ring buffers
  key_t in_key;

  in_key = globalArgs.input_db_key;

  log = multilog_open ("mwa-xc-datacapture-log", 0);
  ctx.log = log;
  multilog_add(log, stderr);

  // create the input HDU
  in_hdu = dada_hdu_create(log);
  dada_hdu_set_key(in_hdu, in_key);
  if (dada_hdu_connect(in_hdu) < 0)
  {
    fprintf (stderr, "main: ERROR: could not connect to input HDU\n");
    return EXIT_FAILURE;
  }

  if (dada_hdu_lock_read(in_hdu) < 0)
  {
    fprintf (stderr, "main: ERROR: could not lock read on input HDU\n");
    return EXIT_FAILURE;
  }

  // set up DADA read client
  client = dada_client_create ();
  client->log = log;
  client->data_block        = in_hdu->data_block;
  client->header_block      = in_hdu->header_block;
  client->open_function     = dada_dbfits_open;
  client->io_function       = dada_dbfits_io;
  client->io_block_function = dada_dbfits_io_block;
  client->close_function    = dada_dbfits_close;
  client->direction         = dada_client_reader;  // actually do both read and write
  client->context = &ctx;

  // main loop
  while (!quit)
  {    
    multilog(client->log, LOG_INFO, "main: dada_client_read()\n");

    if (dada_client_read (client) < 0)
      multilog (log, LOG_ERR, "main: error during transfer\n");
    
    multilog(client->log, LOG_INFO, "main: dada_hdu_unlock_read()\n");
    if (dada_hdu_unlock_read (in_hdu) < 0)
    {
      multilog (log, LOG_ERR, "main: could not unlock read on hdu\n");
      quit = 1;
    }

    if (ctx.internal_error)
    {
      multilog (log, LOG_ERR, "main: internal error ocurred during read\n");
      quit = 1;
    }

    if (quit)
      client->quit = 1;
    else
    {
      multilog(client->log, LOG_INFO, "main: dada_hdu_lock_read()\n");
      
      if (dada_hdu_lock_read (in_hdu) < 0)
      {
        multilog (log, LOG_ERR, "could not lock read on hdu\n");
        return EXIT_FAILURE;
      }
    }

    quit = 1;
  }
    
  multilog(client->log, LOG_INFO, "main: dada_hdu_disconnect()\n");
  if (dada_hdu_disconnect (in_hdu) < 0)
  {
    multilog(log, LOG_ERR, "main: failed to disconnect HDU\n");
    return EXIT_FAILURE;
  }

  // destroy HDUs and read client
  dada_hdu_destroy (in_hdu);  
  dada_client_destroy (client);

  // close log
  multilog_close (log);

  return EXIT_SUCCESS;

  //
  // Do all the things
  //
  fitsfile *fptr;
  fitsfile *fptr_metafits;
  metafits_info mptr;

  int status = 0;
    
  // Open metafits file
  if (open_fits(&fptr_metafits, globalArgs.metafits_path)) {
    log_error("Error openning metafits file.");
    exit(EXIT_FAILURE);
  }

  // Read the metafits into a structure
  if (read_metafits(fptr_metafits, &mptr)) {
    log_error("Error reading metafits file.");
    exit(EXIT_FAILURE);
  }

  // Create new fits file, based on metafits
  // Make a new filename- oooooooooo_YYYYMMDDhhmmss_gpuboxGG_FF.fits
  char filename[44] = "";
  char timestring[15];
  get_time_string_for_fits(timestring);
  int channel = 1;
  int file_number = 0;
  
  sprintf(filename, "%ld_%s_gpubox%02d_%02d.fits", mptr.obsid, timestring, channel, file_number);

  if (create_fits(&fptr, filename, &mptr)) {
    log_error("Error creating new fits file.");
    exit(EXIT_FAILURE);
  }

  // Close the metafits file
  if (close_fits(fptr_metafits)) {
    log_error("Error closing metafits file.");
    exit(EXIT_FAILURE);
  }

  // Close the fits file we created
  if (close_fits(fptr)) {
    log_error("Error closing new fits file.");
    exit(EXIT_FAILURE);
  }

  // Terminate cleanly
  log_info("Exiting...");
  exit(EXIT_SUCCESS);
}
