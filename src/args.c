/**
 * @file args.c
 * @author Greg Sleap
 * @date 21 May 2018
 * @brief This is the code that parses and validates command line arguments
 *
 */
#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include "multilog.h"
#include "args.h"

int process_args(int argc, char* argv[], globalArgs_t* globalArgs)
{
    globalArgs->input_db_key = 0;    
    globalArgs->metafits_path = NULL;
    globalArgs->destination_path = NULL;

    static const char *optString = "k:p:i:d:m:?";

	static const struct option longOpts[] =
	{
		{ "key", required_argument, NULL, 'k' },        
        { "metafits", required_argument, NULL, 'm' },
        { "destination", required_argument, NULL, 'd' },
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
				globalArgs->input_db_key = strtol(optarg, NULL, 16);
				break;

            case 'd':
                globalArgs->destination_path = optarg;
                break;

            case 'm':
                globalArgs->metafits_path = optarg;
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
    if (!globalArgs->input_db_key) {
        fprintf(stderr, "Error: input shared memory key (-k | --key) is mandatory.\n");
        print_usage();
        exit(1);
    }

    if (!globalArgs->metafits_path) {
        fprintf(stderr, "Error: metafits path (-m | --metafits) is mandatory.\n");
        print_usage();
        exit(1);
    }

    if (!globalArgs->destination_path) {
        fprintf(stderr, "Error: destination path (-d | --destination) is mandatory.\n");
        print_usage();
        exit(1);
    }
   
    return EXIT_SUCCESS;
}

void print_usage(void)
{
	printf("\nUsage: mwa_xc_datacapture [OPTION]...\n\n");
	printf("This code will open the dada ringbuffer containing raw \n");
    printf("visibility data from the MWA Crosse Correlator.\n");
    printf("It will then write out a fits file to be picked up by the \n");
    printf("archiver process.\n\n");
	printf("  -k --key=KEY              Hexadecimal shared memory key\n");    
    printf("  -d --destination=PATH     Destination path for gpubox files\n");
    printf("  -m --metafits=PATH        Metafits directory path\n");
	printf("  -? --help                 This help text\n");
}