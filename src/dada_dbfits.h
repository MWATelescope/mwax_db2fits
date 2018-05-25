/*
 * dada_dbfits.h
 *
 *  Created on: 23-May-2018
 *      Author: Greg Sleap
 */
#ifndef XC_DADA_DBFITS_H_
#define XC_DADA_DBFITS_H_

#include <assert.h>

#include "dada_def.h"
#include "dada_client.h"
#include "dada_hdu.h"
#include "multilog.h"

typedef struct {
    int internal_error;

    // PSRDADA stuff
    uint64_t header_size;     // size of the DADA header blocks
    uint64_t block_size;      // size of the DADA data blocks
    multilog_t *log;
    char *curr_block;
    char block_open;          // flag for currently open output HDU
    uint64_t bytes_written;   // number of bytes currently written to output HDU
    uint64_t bytes_read;

} dada_db_t;

// function prototypes
int dada_dbfits_init(dada_db_t *ctx, dada_hdu_t *in);
int dada_dbfits_open(dada_client_t *client);
int dada_dbfits_close(dada_client_t *client, uint64_t bytes_written);
int64_t dada_dbfits_io(dada_client_t *client, void *buffer, uint64_t bytes);
int64_t dada_dbfits_io_block(dada_client_t *client, void *buffer, uint64_t bytes, uint64_t block_id);

#endif