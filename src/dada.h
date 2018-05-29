/*
 * dada.h
 *
 *  Created on: 23-May-2018
 *      Author: Greg Sleap
 */
#ifndef XC_DADA_H_
#define XC_DADA_H_

#include <stdint.h>
#include <sys/types.h> 
#include "fitswriter.h"
#include "multilog.h"
#include "fitsio.h"

#define HAVE_HWLOC      // This is used by the psrdada library to set if we have the hwloc library or not. This lib is used to abstract NUMA / architecture.
typedef __key_t key_t;  // This is needed because my types.h is not pointing at the correct one.

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
    float obs_int_time; 
    long transfer_size;    
} dada_db_t;

#endif