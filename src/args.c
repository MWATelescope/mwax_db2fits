/*
 * args.c
 *
 *  Created on: 21-May-2018
 *      Author: Greg Sleap
 */
#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include "args.h"
#include "log.h"

int process_args(int argc, char* argv[])
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
		switch( opt ) 
        {

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
                return EXIT_FAILURE;

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
  log_debug("Command line options used:");
  log_debug("* Listening on:    %s port %d", globalArgs.listen_interface, globalArgs.listen_port);
  log_debug("* Destination url: %s", globalArgs.destination_url);
  log_debug("* Metafits path:   %s", globalArgs.metafits_path);

  return EXIT_SUCCESS;
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