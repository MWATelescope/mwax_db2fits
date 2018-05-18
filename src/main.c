/*
 * main.c
 *
 *  Created on: 15-May-2018
 *      Author: Greg Sleap
 */
#include <getopt.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include "main.h"
#include "fitsio.h"
#include "fitswriter.h"
#include "log.h"

// Command line Args
struct globalArgs_t
{
    int listen_port;
    char* listen_interface;
    char* destination_url;
    char* metafits_path;
} globalArgs;

int main(int argc, char* argv[])
{	  
	globalArgs.listen_port = -1;
	globalArgs.listen_interface = NULL;
  globalArgs.destination_url = NULL;
  globalArgs.metafits_path = NULL;

  static const char *optString = "p:i:d:m:?";

	static const struct option longOpts[] =
	{
		{ "port", required_argument, NULL, 'p' },
		{ "interface", required_argument, NULL, 'i' },
    { "destination", required_argument, NULL, 'd' },
    { "metafits", required_argument, NULL, 'm' },
	  { "help", no_argument, NULL, '?' },
	  { NULL, no_argument, NULL, 0 }
	};

	int opt = 0;
	int longIndex =  0;

	opt = getopt_long( argc, argv, optString, longOpts, &longIndex );
	while( opt != -1 )
  {
		switch( opt ) {

			case 'p':
				globalArgs.listen_port = atoi(optarg);
				break;

			case 'i':
				globalArgs.listen_interface = optarg;
				break;

      case 'd':
        globalArgs.destination_url = optarg;
        break;

      case 'm':
        globalArgs.metafits_path = optarg;
        break;

			case '?':
				print_usage();
				return 0;

			default:
				/* You won't actually get here. */
				break;
		}

		opt = getopt_long( argc, argv, optString, longOpts, &longIndex );
	}

  // Check that mandatory parameters are passed
  if (!globalArgs.listen_interface) {
    printf("Error: listen_interface (-i) is mandatory.\n");
    print_usage();
    exit(1);
  }

  if (globalArgs.listen_port == -1) {
    printf("Error: listen_port (-p) is mandatory.\n");
    print_usage();
    exit(1);
  }

  if (!globalArgs.destination_url) {
    printf("Error: destination_url (-d) is mandatory.\n");
    print_usage();
    exit(1);
  }

  if (!globalArgs.metafits_path) {
    printf("Error: metafits (-m) is mandatory.\n");
    print_usage();
    exit(1);
  }

  // print all of the options (this is debug)
  printf("Command line options used:\n");
  printf("\tListening on:    %s port %d\n", globalArgs.listen_interface, globalArgs.listen_port);
  printf("\tDestination url: %s\n", globalArgs.destination_url);
  printf("\tMetafits path:   %s\n", globalArgs.metafits_path);

  // Do all the things
  fitsfile *fptr;
  fitsfile *fptr_metafits;
  metafits_info mptr;

  int status = 0;
  const char *filename = "!xyz.fits"; //The ! tells cfitsio to overwrite the file
  
  // Open metafits file
  if (open_fits(&fptr_metafits, globalArgs.metafits_path)) {
    printf("Error openning metafits file.\n");
    exit(EXIT_FAILURE);
  }

  // Read the metafits into a structure
  if (read_metafits(fptr_metafits, &mptr)) {
    printf("Error reading metafits file.\n");
    exit(EXIT_FAILURE);
  }

  // Create new fits file, based on metafits
  if (create_fits(&fptr, filename, &mptr)) {
    printf("Error creating new fits file.\n");
    exit(EXIT_FAILURE);
  }

  // Close the metafits file
  if (close_fits(fptr_metafits)) {
    printf("Error closing metafits file.\n");
    exit(EXIT_FAILURE);
  }

  // Close the fits file we created
  if (close_fits(fptr)) {
    printf("Error closing new fits file.\n");
    exit(EXIT_FAILURE);
  }

  // Terminate cleanly
  printf("Exiting\n");
  exit(EXIT_SUCCESS);
}

void print_usage(void)
{
	printf("\nUsage: mwa_xc_datacapture [OPTION]...\n\n");
	printf("This code will open the specified TCP port on the specified\n");
  printf("interface and listen for data from the MWA Crosse Correlator.\n");
  printf("It will then write out a fits file and send it to the online\n");
  printf("archive NGAS server.\n\n");
	printf("  -p --port=PORT           TCP port to listen on\n");
	printf("  -i --interface=INTERFACE Network interface to listen on\n");
  printf("  -d --destination=URL     URL of the destination NGAS server\n");
  printf("  -m --metafits=PATH       Metafits directory path\n");
	printf("  -? --help                This help text\n");
}
