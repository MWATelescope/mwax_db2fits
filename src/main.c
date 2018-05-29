/**
 * @file main.c
 * @author Greg Sleap
 * @date 15 May 2018
 * @brief The main code block
 *
 * This code performs initialisation and performs the main command loop.
 */
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <signal.h>

#include "args.h"
#include "fitsio.h"
#include "dada_dbfits.h"
#include "multilog.h"

int quit = 0;

globalArgs_t globalArgs;

dada_db_t ctx;

void sig_handler(int signum)
{    
    if (ctx.log != NULL)
      multilog(ctx.log, LOG_INFO, "Received signal %d\n", signum);
    else
      printf("Received signal %d\n", signum);
      
    // If we have things running, tell them to stop!
    quit = 1;
}


int main(int argc, char* argv[])
{	  
  printf("Starting up mwa_xc_datacapture...");
    
  if (process_args(argc, argv))
  {
    exit(EXIT_FAILURE);
  }
  
  // DADA Primary Read Client
  dada_client_t* client = 0;

  // Logger
  multilog_t* logger = 0;

  // DADA Header plus Data Units
  dada_hdu_t* in_hdu = 0;  
  
  // DADA keys for input ring buffers
  key_t in_key;

  in_key = globalArgs.input_db_key;

  logger = multilog_open ("mwa-xc-datacapture-log", 0);
  ctx.log = logger;
  multilog_add(logger, stderr);

  // print all of the options (this is debug)
  multilog(ctx.log, LOG_INFO, "Command line options used:\n");
  multilog(ctx.log, LOG_INFO, "* Shared Memory key:    %x\n", globalArgs.input_db_key);
  multilog(ctx.log, LOG_INFO, "* Listening on:         %s port %d\n", globalArgs.listen_interface, globalArgs.listen_port);
  multilog(ctx.log, LOG_INFO, "* Destination url:      %s\n", globalArgs.destination_url);
  multilog(ctx.log, LOG_INFO, "* Metafits path:        %s\n", globalArgs.metafits_path);

  // Catch SIGINT
  signal(SIGINT, sig_handler);

  // create the input HDU
  in_hdu = dada_hdu_create(logger);
  dada_hdu_set_key(in_hdu, in_key);
  if (dada_hdu_connect(in_hdu) < 0)
  {
    multilog(ctx.log, LOG_ERR, "main: ERROR: could not connect to input HDU\n");
    return EXIT_FAILURE;
  }

  if (dada_hdu_lock_read(in_hdu) < 0)
  {
    multilog(ctx.log, LOG_ERR, "main: ERROR: could not lock read on input HDU\n");
    return EXIT_FAILURE;
  }

  // set up DADA read client
  client = dada_client_create ();
  client->log = ctx.log;
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
    multilog(ctx.log, LOG_INFO, "main: dada_client_read()\n");

    if (dada_client_read (client) < 0)
      multilog(ctx.log, LOG_ERR, "main: error during transfer\n");
    
    if (quit)
      client->quit = 1;
    else
    {
      multilog(ctx.log, LOG_INFO, "main: dada_hdu_unlock_read()\n");
      if (dada_hdu_unlock_read (in_hdu) < 0)
      {
        multilog(ctx.log, LOG_ERR, "main: could not unlock read on hdu\n");
        quit = 1;
      }
        
      multilog(ctx.log, LOG_INFO, "main: dada_hdu_lock_read()\n");
      
      if (dada_hdu_lock_read(in_hdu) < 0)
      {
        multilog (ctx.log, LOG_ERR, "could not lock read on hdu\n");
        return EXIT_FAILURE;
      }
    }    
  }
    
  multilog(ctx.log, LOG_INFO, "main: dada_hdu_disconnect()\n");
  if (dada_hdu_disconnect (in_hdu) < 0)
  {
    multilog(ctx.log, LOG_ERR, "main: failed to disconnect HDU\n");
    return EXIT_FAILURE;
  }

  // destroy HDUs and read client
  dada_hdu_destroy(in_hdu);  
  dada_client_destroy(client);

  // close log
  multilog_close(logger);

  return EXIT_SUCCESS;  
}
