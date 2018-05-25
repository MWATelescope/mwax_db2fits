#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "dada_client.h"
#include "dada_hdu.h"
#include "dada_def.h"
#include "dada_msg.h"
#include "sock.h"
#include "ascii_header.h"
#include "daemon.h"

#include "dada.h"
#include "log.h"

int64_t sock_recv (int fd, char* buffer, uint64_t size, int flags)
{
  int64_t received = 0;
  uint64_t total_received = 0;

  char peek = flags & MSG_PEEK;

  while (size) {

    received = recv (fd, buffer, size, DADA_MSG_FLAGS | flags);
    if (received < 0) {
      perror ("sock_recv recv");
      return -1;
    }
    else if (received == 0) {
#ifdef _DEBUG
      fprintf (stderr, "sock_recv received zero bytes\n");
#endif
      break;
    }
#ifdef _DEBUG
    fprintf (stderr, "received=%"PRIu64"\n", received);
#endif

    if (!peek) {
      size -= received;
      buffer += received;
      total_received += received;
    }
    else if (received == size)
      return received;

  }

  return total_received;
}

static uint64_t total_received = 0;

/*! Pointer to the function that transfers data to/from the target */
int64_t sock_recv_function (dada_client_t* client, 
			    void* data, uint64_t data_size)
{
#ifdef _DEBUG
  fprintf (stderr, "sock_recv_function %p %"PRIu64"\n", data, data_size);
#endif

  int64_t received = sock_recv (client->fd, data, data_size, 0);

  if (received < 0)
    return -1;

  total_received += received;

  if (received < data_size) {

    /* The transmission has been cut short.  Reset transfer_bytes.
       Do not adjust total_received; it is used in sock_close_function */
    if (total_received > client->header_size)
      total_received -= client->header_size;
    else
      total_received = 0;

    client->transfer_bytes = total_received;

    multilog (client->log, LOG_WARNING, "Transfer stopped early at %"PRIu64
	      " bytes\n", client->transfer_bytes);

  }

  return received;
}

/*! Function that closes the data file */
int sock_close_function (dada_client_t* client, uint64_t bytes_written)
{
  /* don't close the socket; just send the header back as a handshake */
  if (ascii_header_set (client->header, "TRANSFER_SIZE", "%"PRIu64,
			bytes_written) < 0)  {
    multilog (client->log, LOG_ERR, "Could not set TRANSFER_SIZE\n");
    return -1;
  }

  total_received = 0;

  if (send (client->fd, client->header, client->header_size, 
	    DADA_MSG_FLAGS) < client->header_size) {
    multilog (client->log, LOG_ERR, 
	      "Could not send acknowledgment Header: %s\n", strerror(errno));
    return -1;
  }

  if (sock_close (client->fd) < 0) {
    multilog (client->log, LOG_ERR, "Could not close socket: %s\n",
              strerror(errno));
    return -1;
  }

  client->fd = -1;
  return 0;
}

static int listen_fd;

/*! Function that opens the data transfer target */
int sock_open_function (dada_client_t* client)
{
  uint64_t hdr_size = 0;
  uint64_t transfer_size = 0;
  int64_t ret = 0;

  assert (client != 0);

  client->fd = sock_accept (listen_fd);
  if (client->fd < 0)  {
    multilog (client->log, LOG_ERR, "Error accepting connection: %s\n",
              strerror(errno));
    return -1;
  }

  ret = sock_recv (client->fd, client->header, client->header_size, MSG_PEEK);

  if (ret < client->header_size)
  {
    multilog (client->log, LOG_ERR, 
	      "recv %d out of %d peek at the Header: %s\n", 
	      ret, client->header_size, strerror(errno));
    return -1;
  }

#ifdef _DEBUG
fprintf (stderr, "peek HEADER START\n%sHEADER END\n", client->header);
#endif

  /* Get the transfer size */
  if (ascii_header_get (client->header, "TRANSFER_SIZE", "%"PRIu64,
			&transfer_size) != 1)  {
    multilog (client->log, LOG_ERR, "Header with no TRANSFER_SIZE\n");
    return -1;
  }

  /* Get the header size */
  if (ascii_header_get (client->header, "HDR_SIZE", "%"PRIu64, &hdr_size) != 1)
  {
    multilog (client->log, LOG_ERR, "Header with no HDR_SIZE\n");
    return -1;
  }

  /* Ensure that the incoming header fits in the client header buffer */
  if (hdr_size > client->header_size) {
    multilog (client->log, LOG_ERR, "HDR_SIZE=%u > Block size=%"PRIu64"\n",
	      hdr_size, client->header_size);
    return -1;
  }

  client->header_size = hdr_size;
  client->optimal_bytes = 1024 * 1024;
  client->transfer_bytes = transfer_size;

#ifdef _DEBUG
  fprintf (stderr, "sock_open_function returns\n");
#endif

  return 0;
}

int main_func(int argc, char **argv)
{
  /* DADA Header plus Data Unit */
  dada_hdu_t* hdu = 0;

  /* DADA Secondary Read Client main loop */
  dada_client_t* client = 0;

  /* DADA Logger */
  multilog_t* log = 0;

  /* port on which to listen for incoming connections */
  int port = DADA_DEFAULT_NICDB_PORT;

  /* Flag set in daemon mode */
  char daemon = 0;

  /* Flag set in verbose mode */
  char verbose = 0;

  /* Quit flag */
  char quit = 0;

  /* hexadecimal shared memory key */
  key_t dada_key = DADA_DEFAULT_BLOCK_KEY;

  int arg = 0;

  while ((arg=getopt(argc,argv,"k:dp:sv")) != -1)
    switch (arg) {

    case 'k':
      if (sscanf (optarg, "%x", &dada_key) != 1) {
        fprintf (stderr,"dada_nicdb: could not parse key from %s\n",optarg);
        return -1;
      }
      break;
      
    case 'd':
      daemon=1;
      break;

    case 'p':
      port = atoi (optarg);
      break;

    case 's':
      quit = 1;
      break;

    case 'v':
      verbose=1;
      break;
      
    default:
      return 0;
      
    }

  log = multilog_open ("dada_nicdb", daemon);

  if (daemon) {
    be_a_daemon ();
    multilog_serve (log, DADA_DEFAULT_NICDB_LOG);
  }
  else
    multilog_add (log, stderr);

  hdu = dada_hdu_create (log);

  dada_hdu_set_key(hdu, dada_key);

  if (dada_hdu_connect (hdu) < 0)
    return EXIT_FAILURE;

  if (dada_hdu_lock_write (hdu) < 0)
    return EXIT_FAILURE;

  client = dada_client_create ();

  client->log = log;

  client->data_block = hdu->data_block;
  client->header_block = hdu->header_block;

  client->open_function  = sock_open_function;
  client->io_function    = sock_recv_function;
  client->close_function = sock_close_function;
  client->direction      = dada_client_writer;

  signal (SIGPIPE, SIG_IGN);

  if (verbose)
    multilog (log, LOG_INFO, "opening socket on port: %d\n", port);
  listen_fd = sock_create (&port);
  if (listen_fd < 0)  {
    multilog (log, LOG_ERR, "Error creating socket: %s\n", strerror(errno));
    return EXIT_FAILURE;
  }

  while (!client->quit) {

    if (dada_client_write (client) < 0)
      multilog (log, LOG_ERR, "Error during transfer\n");

    if (quit) {
      client->quit = 1;
    }

  }

  //if (dada_hdu_unlock_write (hdu) < 0)
  //  return EXIT_FAILURE;

  if (dada_hdu_disconnect (hdu) < 0)
    return EXIT_FAILURE;

  return EXIT_SUCCESS;
}
