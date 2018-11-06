/**
 * @file global.h
 * @author Greg Sleap
 * @date 5 Jul 2018
 * @brief This is the header for anything global 
 *
 */
#pragma once

#include <fitsio.h>
#include <linux/limits.h>
#include <pthread.h>
#include <stdint.h>

#include "fitswriter.h"
#include "multilog.h"

#define HAVE_HWLOC      // This is used by the psrdada library to set if we have the hwloc library or not. This lib is used to abstract NUMA / architecture.

#define MWAX_COMMAND_LEN 32 // Size of the command in PSRDADA header. E.g. "CAPTURE","QUIT","IDLE"
#define UTC_START_LEN 20    // Size of UTC_START in the PSRDADA header (e.g. 2018-08-08-08:00:00)
#define PROJ_ID_LEN 255     // Size of the Project ID used by the MWA metadata database
#define HOST_NAME_LEN 64 
#define IP_AS_STRING_LEN 15 // xxx.xxx.xxx.xxx   

typedef struct dada_db_s {
    // PSRDADA stuff
    uint64_t header_size;     // size of the DADA header blocks
    uint64_t block_size;      // size of the DADA data blocks
    int block_number;
    multilog_t *log;
    char *curr_block;
    char block_open;          // flag for currently open output HDU
    uint64_t bytes_written;   // number of bytes currently written to output HDU
    uint64_t bytes_read;

    // Common
    char hostname[HOST_NAME_LEN+1];

    // FITS info
    char* destination_dir;
    fitsfile *fits_ptr;
    char fits_filename[PATH_MAX];
    int fits_file_number;
    
    // Observation info
    int populated;
    long obs_id;
    long subobs_id;
    char command[MWAX_COMMAND_LEN];
    char utc_start[UTC_START_LEN];
    int obs_offset;
    int nbit;
    int npol;
    int ninputs_xgpu;
    int int_time_msec;
    uint64_t transfer_size;
    char proj_id[PROJ_ID_LEN];
    int exposure_sec;    
    int coarse_channel;
    int corr_coarse_channel;
    int secs_per_subobs;
    long unix_time;
    int unix_time_msec;
    int fine_chan_width_hz;
    int nfine_chan;
    int bandwidth_hz;       
    char multicast_ip[IP_AS_STRING_LEN + 1];
    int multicast_port;
    char multicast_src_ip[IP_AS_STRING_LEN + 1];
    
    // Not from header- calculated values
    uint64_t nbaselines;                        
    int obs_marker_number;               
    int no_of_integrations_per_subobs;
    uint64_t expected_transfer_size_of_one_fine_channel;
    uint64_t expected_transfer_size_of_integration;
    uint64_t expected_transfer_size_of_integration_plus_weights;
    uint64_t expected_transfer_size_of_subobs;
    uint64_t expected_transfer_size_of_subobs_plus_weights;   
} dada_db_s;

int initialise_quit();
int set_quit(int value);
int get_quit();
int destroy_quit();