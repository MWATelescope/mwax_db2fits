#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include "common.h"

int write_header(char *header_filename, char *output_filename, int *output_file)
{
    char header_buffer[HEADER_LEN] = {0x00};

    // read header
    int header_file = open(header_filename, O_RDONLY);
    if (header_file < 0)
    {
        printf("Failed to read header file %s.\n", header_filename);
        return EXIT_FAILURE;
    }
    int bytes_read = read(header_file, header_buffer, HEADER_LEN);
    close(header_file);
    printf("%d bytes read from %s.\n", bytes_read, header_filename);

    // Write data file to std out
    *output_file = open(output_filename, O_WRONLY + O_CREAT, S_IRUSR + S_IWUSR);

    if (*output_file < 0)
    {
        printf("Error opening destination file for writing: %s.\n", output_filename);
        exit(EXIT_FAILURE);
    }

    int bytes_written = write(*output_file, header_buffer, HEADER_LEN);

    if (bytes_written != HEADER_LEN)
    {
        printf("Write of header to %s failed.  Returned a value of %d.\n", output_filename, bytes_written);
        exit(EXIT_FAILURE);
    }

    printf("%d bytes written to header of %s.\n", bytes_written, output_filename);

    return EXIT_SUCCESS;
}

int write_visibilities_hdu(int output_file, int nbaselines, int nfinechan, int npols, int nvalues, int timestep, int start_number)
{
    int buffer_len = nbaselines * nfinechan * npols * nvalues;
    int buffer_bytes = buffer_len * sizeof(float);
    float *buffer = calloc(buffer_len, sizeof(float));

    // Fill buffer with values
    for (int n = 0; n < buffer_len; n++)
    {
        buffer[n] = (float)n + (float)start_number;
    }

    // Write to disk
    int bytes_written = write(output_file, buffer, buffer_bytes);

    // Check
    if (bytes_written != buffer_bytes)
    {
        printf("Error writing visibilities (timestep: %d). Wrote %d bytes- should have been %d.\n", timestep, bytes_written, buffer_bytes);
        return EXIT_FAILURE;
    }
    printf("%d bytes written to visibilities (timestep: %d).\n", bytes_written, timestep);
    return EXIT_SUCCESS;
}

int write_weights_hdu(int output_file, int nbaselines, int nfinechan, int npols, int nvalues, int timestep, float start_number, float increment)
{
    int buffer_len = nbaselines * npols;
    int buffer_bytes = buffer_len * sizeof(float);
    float *buffer = calloc(buffer_len, sizeof(float));

    // Fill buffer with values
    for (int n = 0; n < buffer_len; n++)
    {
        buffer[n] = start_number + ((float)n * increment);
    }

    // Write to disk
    int bytes_written = write(output_file, buffer, buffer_bytes);

    // Check
    if (bytes_written != buffer_bytes)
    {
        printf("Error writing weights (timestep: %d). Wrote %d bytes- should have been %d.\n", timestep, bytes_written, buffer_bytes);
        return EXIT_FAILURE;
    }
    printf("%d bytes written to weights (timestep: %d).\n", bytes_written, timestep);
    return EXIT_SUCCESS;
}