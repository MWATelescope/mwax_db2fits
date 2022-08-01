#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#include "../common.h"

void usage()
{
    printf("make_test01_data header output_file\n"
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
    if ((argc - optind) != 2)
    {
        printf("ERROR: header and output file must be specified\n");
        usage();
        exit(EXIT_FAILURE);
    }

    char *header_filename = strdup(argv[optind]);
    char *output_filename = strdup(argv[optind + 1]);

    int output_file = 0;

    write_header(header_filename, output_filename, &output_file);

    // Create the visibilities data
    int ntimesteps = 2;
    int ntiles = 2;
    int nbaselines = (ntiles * (ntiles + 1)) / 2;
    int nfinechan = 2;
    int npols = 4;   // xx,xy,yx,yy
    int nvalues = 2; // r,i

    for (int timestep = 1; timestep <= ntimesteps; timestep++)
    {
        // Write visibilities
        if (write_visibilities_hdu(output_file, nbaselines, nfinechan, npols, nvalues, timestep, ((timestep * 2) - 2) * 100) != EXIT_SUCCESS)
        {
            exit(EXIT_FAILURE);
        }

        // Write weights
        if (write_weights_hdu(output_file, nbaselines, nfinechan, npols, nvalues, timestep, ((timestep * 2) - 1) * 100) != EXIT_SUCCESS)
        {
            exit(EXIT_FAILURE);
        }
    }

    close(output_file);

    return EXIT_SUCCESS;
}