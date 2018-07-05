/**
 * @file health.h
 * @author Greg Sleap
 * @date 4 Jul 2018
 * @brief This is the header for the code that provides health information to M&C
 *
 */
#ifndef XC_HEALTH_H_
#define XC_HEALTH_H_

#include "multilog.h"

#pragma pack(1)
typedef struct
{    
    multilog_t* log;
    int status;

    // Global stats
    uint64_t hdr_bufsz;
    //int hdr_nbufs;
    // uint64_t hdr_bytes;

    // uint64_t data_bufsz;
    // int data_nbufs;
    // uint64_t data_bytes;
    // int n_readers;

    // // per reader stats
    // uint64_t hdr_bufs_written;
    // uint64_t hdr_bufs_read;
    // uint64_t hdr_full_bufs;
    // uint64_t hdr_clear_bufs;
    // uint64_t hdr_available_bufs;
    
    // uint64_t data_bufs_written;
    // uint64_t data_bufs_read;
    // uint64_t data_full_bufs;
    // uint64_t data_clear_bufs;
    // uint64_t data_available_bufs;
} health_thread_args_t;

#define HEALTH_SLEEP_SECONDS 1  // How often does the health thread send data?

void* health_thread_fn(void *args);

#endif