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

// Keywords within the PSRDADA Header
#define HEADER_POPULATED "POPULATED"                        // 0=header being assembled; 1=header is ready for reading
#define HEADER_OBS_ID "OBS_ID"                              // Observation ID (GPS start time of obs)
#define HEADER_SUBOBS_ID "SUBOBS_ID"                        // GPS start time of this 8 second sub observation
#define HEADER_COMMAND "COMMAND"                            // Command to run: CAPTURE, IDLE, QUIT
#define HEADER_UTC_START "UTC_START"                        // UTC start time of observation (string)
#define HEADER_OBS_OFFSET "OBS_OFFSET"                      // Seconds since start of observation; e.g. 0,8,16,etc
#define HEADER_NBIT "NBIT"                                  // Bits per value (nominally 32 for float)
#define HEADER_NPOL "NPOL"                                  // Number of polarisations (nominally 2)
#define HEADER_NTIMESAMPLES "NTIMESAMPLES"                  // How many high time resolution (VCS) samples do we get (per sec?)
#define HEADER_NINPUTS "NINPUTS"                            // Number of inputs (tiles*pols) which were received by the vcs machines
#define HEADER_NINPUTS_XGPU "NINPUTS_XGPU"                  // Number of inputs (tiles*pols) rounded up to the nearest 16 sent to xGPU
#define HEADER_METADATA_BEAMS "METADATA_BEAMS"              // How many beams to form?
#define HEADER_APPLY_WEIGHTS "APPLY_WEIGHTS"                // Does precorrelator apply weights?
#define HEADER_APPLY_DELAYS "APPLY_DELAYS"                  // Does precorrelator apply delays?
#define HEADER_INT_TIME_MSEC "INT_TIME_MSEC"                // Correlator mode: integrations every x milliseconds
#define HEADER_FSCRUNCH_FACTOR "FSCRUNCH_FACTOR"            // How many 125 Hz ultra fine channels do we average together
#define HEADER_TRANSFER_SIZE "TRANSFER_SIZE"                // Number of bytes of data to expect in this observation:
                                                            // == baselines * (finechannels+1) * (pols^2) * (real_bytes + imaginary_bytes)
                                                            // ==((NINPUTS_XGPU *(NINPUTS_XGPU+2))/8)*(NFINE_CHAN+1)*(NPOL^2)*(NBIT*2/8)
#define HEADER_PROJ_ID "PROJ_ID"                            // Project code for this observation
#define HEADER_EXPOSURE_SEC "EXPOSURE_SEC"                  // Duration of the observation in seconds (always a factor of 8)
#define HEADER_COARSE_CHANNEL "COARSE_CHANNEL"              // Coarse channel number 
#define HEADER_SECS_PER_SUBOBS "SECS_PER_SUBOBS"            // How many seconds are in a sub observation
#define HEADER_UNIXTIME "UNIXTIME"                          // Unix time in seconds
#define HEADER_UNIXTIME_MSEC "UNIXTIME_MSEC"                // Milliseconds portion of Unix time (0-999)
#define HEADER_FINE_CHAN_WIDTH_HZ "FINE_CHAN_WIDTH_HZ"      // Width of fine channels post correlator (kHz)
#define HEADER_NFINE_CHAN "NFINE_CHAN"                      // How many fine channels per coarse channel
#define HEADER_BANDWIDTH_HZ "BANDWIDTH_HZ"                  // Bandwidth of a coarse channel

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
    long transfer_size;
    char proj_id[PROJ_ID_LEN];
    int exposure_sec;    
    int coarse_channel;
    int secs_per_subobs;
    long unix_time;
    int unix_time_msec;
    int fine_chan_width_hz;
    int nfine_chan;
    int bandwidth_hz;          
    
    // Not from header
    int nbaselines;                        
    int obs_marker_number;                     
} dada_db_s;

int initialise_quit();
int set_quit(int value);
int get_quit();
int destroy_quit();