/*
 * args.c
 *
 *  Created on: 21-May-2018
 *      Author: Greg Sleap
 */
#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include "multilog.h"
#include "args.h"

int process_args(int argc, char* argv[])
{
    globalArgs.input_db_key = 0;
    globalArgs.listen_port = -1;
	globalArgs.listen_interface = NULL;
    globalArgs.destination_url = NULL;
    globalArgs.metafits_path = NULL;

    static const char *optString = "k:p:i:d:m:?";

	static const struct option longOpts[] =
	{
		{ "key", required_argument, NULL, 'k' },
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
            case 'k':
				globalArgs.input_db_key = strtol(optarg, NULL, 16);
				break;

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
    if (!globalArgs.input_db_key) {
        fprintf(stderr, "Error: input shared memory key (-k | --key) is mandatory.\n");
        print_usage();
        exit(1);
    }
    
    if (!globalArgs.listen_interface) {
        fprintf(stderr, "Error: listen interface (-i | --listen_interface) is mandatory.\n");
        print_usage();
        exit(1);
    }
        
    if (globalArgs.listen_port == -1) {
        fprintf(stderr, "Error: port (-p | --listen_port) is mandatory.\n");
        print_usage();
        exit(1);
    }

    if (!globalArgs.destination_url) {
        fprintf(stderr, "Error: destination url (-d | --destination_url) is mandatory.\n");
        print_usage();
        exit(1);
    }

    if (!globalArgs.metafits_path) {
        fprintf(stderr, "Error: metafits file (-m | --metafits) is mandatory.\n");
        print_usage();
        exit(1);
    }
    
    return EXIT_SUCCESS;
}

void print_usage(void)
{
	printf("\nUsage: mwa_xc_datacapture [OPTION]...\n\n");
	printf("This code will open the specified TCP port on the specified\n");
    printf("interface and listen for data from the MWA Crosse Correlator.\n");
    printf("It will then write out a fits file and send it to the online\n");
    printf("archive NGAS server.\n\n");
	printf("  -k --key=KEY             Hexadecimal shared memory key\n");
    printf("  -p --port=PORT           TCP port to listen on\n");
	printf("  -i --interface=INTERFACE Network interface to listen on\n");
    printf("  -d --destination=URL     URL of the destination NGAS server\n");
    printf("  -m --metafits=PATH       Metafits directory path\n");
	printf("  -? --help                This help text\n");
}