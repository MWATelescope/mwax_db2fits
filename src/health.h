/**
 * @file health.h
 * @author Greg Sleap
 * @date 4 Jul 2018
 * @brief This is the header for the code that sends health information to M&C via UDP
 *
 */
#pragma once

#include "global.h"
#include "multilog.h"

#pragma pack(push, 1)
typedef struct
{
    int version_major;
    int version_minor;
    int version_build;
    char hostname[HOST_NAME_LEN]; // Hostname mwax_db2fits is running on e.g. "mwax01". HOST_NAME_LEN=64.
    time_t start_time;            // UNIX time mwax_db2fits was started
    time_t health_time;           // UNIX time this health packet was generated
    double up_time;               // Seconds

    int status;     // 1=RUNNING, 2=SHUTTING_DOWN
    long obs_id;    // Obsid of current obs or 0 if not currently processing an observation
    long subobs_id; // Subobsid of current subobs or 0 if not currently processing a sub observation
    float weights_per_tile_x[NTILES_MAX];
    float weights_per_tile_y[NTILES_MAX];

} health_udp_data_s;
#pragma pack(pop)

#define HEALTH_SLEEP_SECONDS 1 // How often does the health thread send data?

void *health_thread_fn(void *args);