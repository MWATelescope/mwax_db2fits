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

#define UTC_START_LEN 20    // Size of UTC_START in the PSRDADA header (e.g. 2018-08-08-08:00:00)
#define PROJ_ID_LEN 255     // Size of the Project ID used by the MWA metadata database

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

    // FITS info
    char* destination_dir;
    fitsfile *fits_ptr;
    char fits_filename[PATH_MAX];
    int fits_file_number;
    
    // Observation info
    long obs_id;                    // ObsID (gps start time of observation)
    char utc_start[UTC_START_LEN];  // UTC Start time of the observation 2018-08-08-08:00:00
    char obs_proj_id[PROJ_ID_LEN];  // Project Code
    int obs_channel_number;         // Coarse Channel number (1-24)
    int obs_duration_sec;           // Duration (seconds) for this whole observation
    int obs_tiles;                  // Number of tiles involved in ths observation
    int obs_bandwidth_khz;          // Bandwidth in kHz of a coarse channel (e.g. 1280)
    int obs_pols;                   // Number of polarisations from each tile (X,Y)
    int obs_freq_res_khz;           // Frequency averaging / resolution for this observation (kHz)
    int obs_baselines;              // Number of baselines: (tiles*(tiles+1) / 2)
    int obs_fine_channels;          // Number of fine channels in this coarse channel
    int obs_int_time_msec;          // Integration time (msec)
    int obs_subobs_sec;             // Seconds per sub observation (should be fixed at 8)
    long transfer_size_bytes;       // How many bytes are expected in this sub observation
    int obs_marker_number;          // 0 based index of block number within this 8 sec subobds (used by the final fits file)
    int obs_offset_sec;             // 0 based seconds offset from start of obs. 0 (sec) = this is the first 8 sec subobs, 8 (sec) = this is the 2nd 8 sec sub obs.
    int nbit;                       // bits per sample e.g. 32 for a float
    long unix_time;                 // UNIX time
    int unix_time_msec;             // Milliseconds offset of UNIX time    
} dada_db_s;

int initialise_quit();
int set_quit(int value);
int get_quit();
int destroy_quit();