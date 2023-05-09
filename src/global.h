/**
 * @file global.h
 * @author Greg Sleap
 * @date 5 Jul 2018
 * @brief This is the header for anything global
 *
 */
#pragma once

#define HAVE_HWLOC // This is used by the psrdada library to set if we have the hwloc library or not. This lib is used to abstract NUMA / architecture.

#include <fitsio.h>
#include <linux/limits.h>
#include <pthread.h>
#include <stdint.h>
#include "fitswriter.h"
#include "multilog.h"

#define STATUS_OFFLINE 0
#define STATUS_RUNNING 1
#define STATUS_SHUTTING_DOWN 2

#define TEMP_FITS_FILENAME_LEN PATH_MAX                // Max length the temp fits filename (and path) can be. This is 4 chars more than the fits filename
#define FITS_FILENAME_LEN (TEMP_FITS_FILENAME_LEN - 4) // Max length the fits filename (and path) can be
#define DEFAULT_FILE_SIZE_LIMIT 10737418240l           // Default file size limit- 10GB
#define MWAX_VERSION_STRING_LEN 11                     // Size of version strings for mwax_u2s, mwax_db2correlate2db and mwax_db2fits
#define MWAX_MODE_LEN 33                               // Size of the MODE value in PSRDADA header. E.g. "HW_LFILES","VOLTAGE_START","NO_CAPTURE", "QUIT"
#define UTC_START_LEN 21                               // Size of UTC_START in the PSRDADA header (e.g. 2018-08-08-08:00:00)
#define PROJ_ID_LEN 256                                // Size of the Project ID used by the MWA metadata database
#define HOST_NAME_LEN 64                               // Length of hostname
#define IP_AS_STRING_LEN 16                            // xxx.xxx.xxx.xxx
#define COARSE_CHANNEL_MAX 255                         // Highest possible coarse channel number
#define INT_TIME_MSEC_MIN 200                          // Minimum integration time (milliseconds)

typedef struct
{
    multilog_t *log;
    ipcbuf_t *header_block;
    ipcbuf_t *data_block;
    int status;
    char *health_udp_interface;
    char *health_udp_interface_ip;
    char *health_udp_ip;
    int health_udp_port;
    char hostname[HOST_NAME_LEN];

    // Data that changes during main loop
    long obs_id;
    long subobs_id;
    // The weights are in the dada datablock in after the visibilties.
    // Here we keep 2 arrays of floats, 1 element per tile
    // and accumulate the weights and increment the weights
    // counter each time we accumulate. We want to get the tile weights,
    // so we cheat and use the XX and YY (the weights given to mwax_db2fits
    // are for baselines, not tiles).
    // Then each health tick, we get the average weight for xx and yy
    // (using weights_counter as the denominator) and send this with
    // the health packet.
    float *weights_per_tile_xx;
    float *weights_per_tile_yy;
    int weights_counter;
} health_thread_data_s;

typedef struct dada_db_s
{
    // PSRDADA stuff
    uint64_t header_size; // size of the DADA header blocks
    uint64_t block_size;  // size of the DADA data blocks
    int block_number;
    multilog_t *log;
    char *curr_block;
    char block_open;        // flag for currently open output HDU
    uint64_t bytes_written; // number of bytes currently written to output HDU
    uint64_t bytes_read;

    // Common
    char hostname[HOST_NAME_LEN];

    // Version info
    char mwax_u2s_version[MWAX_VERSION_STRING_LEN];
    char mwax_db2correlate2db_version[MWAX_VERSION_STRING_LEN];

    // FITS info
    char *destination_dir;
    fitsfile *fits_ptr;
    char fits_filename[PATH_MAX - 4]; // we subtract 4 so we ensure temp_fits_filename can fit fits_filename + '.tmp'
    char temp_fits_filename[PATH_MAX];
    int fits_file_number;
    long fits_file_size;
    long fits_file_size_limit;

    // Observation info
    int populated;
    long obs_id;
    long subobs_id;
    char mode[MWAX_MODE_LEN];
    char utc_start[UTC_START_LEN];
    int obs_offset;
    int nbit;
    int npol;
    int ninputs;
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
    char multicast_ip[IP_AS_STRING_LEN];
    int multicast_port;
    int fscrunch_factor;

    // Not from header- calculated values
    uint64_t nbaselines;
    int obs_marker_number;
    int no_of_integrations_per_subobs;
    uint64_t expected_transfer_size_of_one_fine_channel;
    uint64_t expected_transfer_size_of_weights;
    uint64_t expected_transfer_size_of_integration;
    uint64_t expected_transfer_size_of_integration_plus_weights;
    uint64_t expected_transfer_size_of_subobs;
    uint64_t expected_transfer_size_of_subobs_plus_weights;
} dada_db_s;

// Methods for the Quit mutex
int quit_init();
int quit_set(int value);
int quit_get();
int quit_destroy();

// Methods for managing data which will be used eventually to send health packets
int health_manager_init();
int health_manager_set_info(int status, long obs_id, long subobs_id);
int health_manager_get_info(int *status, long *obs_id, long *subobs_id, float **weights_per_tile_xx, float **weights_per_tile_yy);
int health_manager_set_weights_info(float *buffer);
int health_manager_reset_health_weights_info();
int health_manager_destroy();

// Method for compression mode
const char *compression_mode_name(int compression_mode);

// Ensure these global vars only get create once for the entire program (not per compile unit)
#ifndef GLOBAL_H
#define GLOBAL_H
extern pthread_mutex_t g_quit_mutex;
extern int g_quit;

extern pthread_mutex_t g_health_manager_mutex;
extern health_thread_data_s g_health_manager;

extern dada_db_s g_ctx;
#endif