/**
 * @file global.h
 * @author Greg Sleap
 * @date 5 Jul 2018
 * @brief This is the header for anything global 
 *
 */
#ifndef XC_GLOBAL_H_
#define XC_GLOBAL_H_

#include <pthread.h>
#include <fitsio.h>
#include <stdint.h>

#include "fitswriter.h"
#include "multilog.h"

#define HAVE_HWLOC      // This is used by the psrdada library to set if we have the hwloc library or not. This lib is used to abstract NUMA / architecture.

typedef struct {
    // PSRDADA stuff
    uint64_t header_size;     // size of the DADA header blocks
    uint64_t block_size;      // size of the DADA data blocks
    int block_number;
    multilog_t *log;
    char *curr_block;
    char block_open;          // flag for currently open output HDU
    uint64_t bytes_written;   // number of bytes currently written to output HDU
    uint64_t bytes_read;

    // FITS info
    char* destination_dir;
    fitsfile *fits_ptr;
    char fits_filename[44];
    
    // Observation info
    long obs_id;
    char obs_proj_id[5];
    int obs_channel;
    int obs_duration;
    int obs_tiles;
    int obs_bandwidth;
    int obs_pols;
    int obs_freq_res;
    int obs_baselines;
    int obs_fine_channels;
    float obs_int_time; 
    int obs_secs_per_subobs;
    long transfer_size; 
    int obs_marker;   
} dada_db_s;

int initialise_quit();
int set_quit(int value);
int get_quit();
int destroy_quit();

#endif