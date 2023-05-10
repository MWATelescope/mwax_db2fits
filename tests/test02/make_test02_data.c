#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#include "../common.h"

#define NTIMESTEPS 2
#define NTILES 2
#define NBASELINES ((NTILES * (NTILES + 1)) / 2)
#define NFINECHAN 2
#define NPOLS 4   // xx,xy,yx,yy
#define NVALUES 2 // r,i

void usage()
{
    printf("make_test02_data subobs_number header output_file\n"
           "subobs_number subobs number (1-based) e.g. 1,2...\n"
           "header        DADA header file contain obs metadata\n"
           "output_file   Output data filename\n");
}

int main(int argc, char **argv)
{
    // Process args
    int arg = 0;

    while ((arg = getopt(argc, argv, "h:")) != -1)
    {
        switch (arg)
        {
        default:
            usage();
            return 0;
        }
    }

    // check the header file was supplied
    if ((argc - optind) != 3)
    {
        printf("ERROR: subobs_number, header and output file must be specified\n");
        usage();
        exit(EXIT_FAILURE);
    }

    int subobs_number = atoi(argv[optind]);
    char *header_filename = strdup(argv[optind + 1]);
    char *output_filename = strdup(argv[optind + 2]);

    int output_file = 0;

    write_header(header_filename, output_filename, &output_file);

    // Create the visibilities data
    for (int timestep = 1; timestep <= NTIMESTEPS; timestep++)
    {
        // Write visibilities
        if (write_visibilities_hdu(output_file, NBASELINES, NFINECHAN, NPOLS, NVALUES, timestep, (((subobs_number - 1) * NTIMESTEPS) + timestep) * 101) != EXIT_SUCCESS)
        {
            exit(EXIT_FAILURE);
        }

        // Write weights
        if (write_weights_hdu(output_file, NBASELINES, NFINECHAN, NPOLS, NVALUES, timestep, (((subobs_number - 1) * NTIMESTEPS) + (timestep - 1)) * 0.04, 0.04) != EXIT_SUCCESS)
        {
            exit(EXIT_FAILURE);
        }
    }

    close(output_file);

    return EXIT_SUCCESS;
}