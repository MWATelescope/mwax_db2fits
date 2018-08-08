/**
 * @file health.h
 * @author Greg Sleap
 * @date 4 Jul 2018
 * @brief This is the header for the code that provides health information to M&C
 *
 */
#pragma once

#include "multilog.h"
#include "dada_client.h"

typedef struct
{    
    multilog_t* log;
    ipcbuf_t* header_block;
    ipcbuf_t* data_block;
    int status;
    char* health_udp_ip;
    int health_udp_port;
} health_thread_args_s;

#pragma pack(push, 1)
typedef struct 
{
    int32_t status;
    
    // Global stats
    uint64_t hdr_bufsz;
    int32_t hdr_nbufs;
    uint64_t hdr_bytes;

    uint64_t data_bufsz;
    int32_t data_nbufs;
    uint64_t data_bytes;
    int32_t n_readers;

    // per reader stats
    uint64_t hdr_bufs_written;
    uint64_t hdr_bufs_read;
    uint64_t hdr_full_bufs;
    uint64_t hdr_clear_bufs;
    uint64_t hdr_available_bufs;
    
    uint64_t data_bufs_written;
    uint64_t data_bufs_read;
    uint64_t data_full_bufs;
    uint64_t data_clear_bufs;
    uint64_t data_available_bufs;
} health_data_s;
#pragma pack(pop)

#define HEALTH_SLEEP_SECONDS 1  // How often does the health thread send data?

void* health_thread_fn(void *args);