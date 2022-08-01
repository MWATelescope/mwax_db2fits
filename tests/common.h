#define HEADER_LEN 4096

int write_header(char *header_filename, char *output_filename, int *output_file);
int write_visibilities_hdu(int output_file, int nbaselines, int nfinechan, int npols, int nvalues, int timestep, int start_number);
int write_weights_hdu(int output_file, int nbaselines, int nfinechan, int npols, int nvalues, int timestep, int start_number);