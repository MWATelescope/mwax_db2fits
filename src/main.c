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
#include <linux/limits.h>
#include <stdio.h>
#include <signal.h>
#include <pthread.h>
#include <unistd.h>

#include "args.h"
#include "dada_dbfits.h"
#include "fitsio.h"
#include "global.h"
#include "health.h"
#include "multilog.h"

#define STATUS_OFFLINE 0
#define STATUS_RUNNING 1
#define STATUS_SHUTTING_DOWN 2

pthread_mutex_t g_quit_mutex;
int g_quit = 0;
dada_db_s g_ctx;

/**
 * 
 *  @brief This captures the relevant signal and set the g_quit variable so the program quits gracefully.
 *  @param[in] signum Signal number to handle.
  */
void sig_handler(int signum)
{    
  if (g_ctx.log != NULL)
    multilog(g_ctx.log, LOG_INFO, "Received signal %d\n", signum);
  else
    printf("Received signal %d\n", signum);
    
  // If we have things running, tell them to stop!
  set_quit(1);
}

/**
 * 
 *  @brief This is main, duh!
 *  @param[in] argc Count of arguments passed in from command line.
 *  @param[in] argv Array of arguments passed in from command line.
 *  @returns EXIT_SUCCESS on success, or any other value if there was an error. 
 */
int main(int argc, char* argv[])
{	  
  // Logger
  multilog_t* logger = 0;
  logger = multilog_open ("mwa-xc-datacapture-log", 0);
  multilog_add(logger, stderr);
  multilog(logger, LOG_INFO, "Starting mwax_db2fits...\n");
    
  globalArgs_s globalArgs;

  if (process_args(argc, argv, &globalArgs))
  {
    exit(EXIT_FAILURE);
  }
  
  // DADA Primary Read Client
  dada_client_t* client = 0;

  // DADA Header plus Data Units
  dada_hdu_t* in_hdu = 0;  
  
  // DADA keys for input ring buffers
  key_t in_key = globalArgs.input_db_key;
  
  g_ctx.log = logger;  

  // Get the current hostname  
  if (gethostname(g_ctx.hostname, HOST_NAME_LEN + 1) != 0)
  {
    multilog(g_ctx.log, LOG_ERR, "main: ERROR: gethostname() failed\n");
    return EXIT_FAILURE;
  }

  multilog(g_ctx.log, LOG_INFO, "Hostname: %s\n", g_ctx.hostname);

  // print all of the options (this is debug)
  multilog(g_ctx.log, LOG_INFO, "Command line options used:\n");
  multilog(g_ctx.log, LOG_INFO, "* Shared Memory key:    %x\n", globalArgs.input_db_key);
  multilog(g_ctx.log, LOG_INFO, "* Metafits path:        %s\n", globalArgs.metafits_path);
  multilog(g_ctx.log, LOG_INFO, "* Destination path:     %s\n", globalArgs.destination_path);
  multilog(g_ctx.log, LOG_INFO, "* Health UDP IP:        %s\n", globalArgs.health_ip);
  multilog(g_ctx.log, LOG_INFO, "* Health UDP Port:      %d\n", globalArgs.health_port);

  // This tells us if we need to quit
  int quit = 0;
  initialise_quit(); // Setup quit mutex
  set_quit(quit);

  // Catch SIGINT
  multilog(g_ctx.log, LOG_INFO, "main(): Configured to catching SIGINT.\n");
  signal(SIGINT, sig_handler);

  // create the input HDU
  multilog(g_ctx.log, LOG_INFO, "main(): Creating HDU handle...\n");
  in_hdu = dada_hdu_create(logger);

  multilog(g_ctx.log, LOG_INFO, "main(): Assigning key %x to HDU handle.\n", globalArgs.input_db_key);
  dada_hdu_set_key(in_hdu, in_key);

  multilog(g_ctx.log, LOG_INFO, "main(): Connecting to HDU with key %x...\n", globalArgs.input_db_key);
  if (dada_hdu_connect(in_hdu) < 0)
  {
    multilog(g_ctx.log, LOG_ERR, "main: ERROR: could not connect to input HDU\n");
    return EXIT_FAILURE;
  }

  multilog(g_ctx.log, LOG_INFO, "main(): Locking HDU %x handle for read...\n", globalArgs.input_db_key);
  if (dada_hdu_lock_read(in_hdu) < 0)
  {
    multilog(g_ctx.log, LOG_ERR, "main: ERROR: could not lock read on input HDU\n");
    return EXIT_FAILURE;
  }

  // Pass stuff to the context
  g_ctx.destination_dir = globalArgs.destination_path;
  
  // set up DADA read client
  multilog(g_ctx.log, LOG_INFO, "main(): Creating DADA client...\n", globalArgs.input_db_key);
  client = dada_client_create ();
  client->log = g_ctx.log;
  client->data_block        = in_hdu->data_block;
  client->header_block      = in_hdu->header_block;
  client->open_function     = dada_dbfits_open;
  client->io_function       = dada_dbfits_io;
  client->io_block_function = dada_dbfits_io_block;
  client->close_function    = dada_dbfits_close;
  client->direction         = dada_client_reader;    
  client->context = &g_ctx;
  
  // Set some useful params based on our ringbuffer config  
  g_ctx.block_size = ipcbuf_get_bufsz((ipcbuf_t *)(client->data_block));
  multilog(g_ctx.log, LOG_INFO, "main(): Block size (one integration) is %lu bytes.\n", g_ctx.block_size);

  // Launch Health thread  
  pthread_t health_thread;

  health_thread_args_s health_args;

  // Zero the structure
  memset(&health_args, 0, sizeof(health_args));
  
  health_args.log = logger;  
  health_args.status = STATUS_RUNNING;
  health_args.header_block = client->header_block;
  health_args.data_block = (ipcbuf_t*)client->data_block;
  health_args.health_udp_ip = globalArgs.health_ip;
  health_args.health_udp_port = globalArgs.health_port;
  
  multilog(g_ctx.log, LOG_INFO, "main():Launching health thread...\n");
  pthread_create(&health_thread, NULL, health_thread_fn, (void*)&health_args);    

  // main loop
  while (!quit)
  {    
    multilog(g_ctx.log, LOG_INFO, "main: dada_client_read()\n");
    
    if (dada_client_read(client) < 0)
    {
      multilog(g_ctx.log, LOG_ERR, "main: error during transfer\n");
      set_quit(1);
    }
    
    // Check quit status
    quit = get_quit();

    if(quit)
    {      
      health_args.status = STATUS_SHUTTING_DOWN;
      client->quit = 1;
    }
    else
    {      
      multilog(g_ctx.log, LOG_INFO, "main: dada_hdu_unlock_read()\n");
      if (dada_hdu_unlock_read (in_hdu) < 0)
      {
        multilog(g_ctx.log, LOG_ERR, "main: could not unlock read on hdu\n");
        return EXIT_FAILURE;
      }
        
      multilog(g_ctx.log, LOG_INFO, "main: dada_hdu_lock_read()\n");
      
      if (dada_hdu_lock_read(in_hdu) < 0)
      {
        multilog (g_ctx.log, LOG_ERR, "could not lock read on hdu\n");
        return EXIT_FAILURE;
      }
    }    
  }

  // Wait for health thread to terminate  
  pthread_join(health_thread, NULL);  

  multilog(g_ctx.log, LOG_INFO, "main: dada_hdu_disconnect()\n");
  if (dada_hdu_disconnect (in_hdu) < 0)
  {
    multilog(g_ctx.log, LOG_ERR, "main: failed to disconnect HDU\n");
    return EXIT_FAILURE;
  }

  // destroy HDUs and read client
  dada_hdu_destroy(in_hdu);  
  dada_client_destroy(client);

  // close log
  multilog_close(logger);

  // Destroy mutexes
  destroy_quit();

  return EXIT_SUCCESS;  
}
