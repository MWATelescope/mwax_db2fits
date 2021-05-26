/**
 * @file health.h
 * @author Greg Sleap
 * @date 4 Jul 2018
 * @brief This is the header for the code that provides health information to M&C
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
    char hostname[HOST_NAME_LEN];
    time_t start_time;
    time_t health_time;
    double up_time;

    int status;
    long obs_id;
    long subobs_id;
} health_udp_data_s;
#pragma pack(pop)

#define HEALTH_SLEEP_SECONDS 1 // How often does the health thread send data?

void *health_thread_fn(void *args);