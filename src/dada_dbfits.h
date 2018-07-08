/**
 * @file dada_dbfits.h
 * @author Greg Sleap
 * @date 23 May 2018
 * @brief This is the header for the code that drives the ring buffers
 *
 */
#ifndef XC_DADA_DBFITS_H_
#define XC_DADA_DBFITS_H_

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "ascii_header.h"
#include "dada_def.h"
#include "dada_client.h"
#include "dada_hdu.h"
#include "multilog.h"
#include "dada.h"

// function prototypes
int dada_dbfits_init(dada_db_s *ctx, dada_hdu_t *in);
int dada_dbfits_open(dada_client_t *client);
int dada_dbfits_close(dada_client_t *client, uint64_t bytes_written);
int64_t dada_dbfits_io(dada_client_t *client, void *buffer, uint64_t bytes);
int64_t dada_dbfits_io_block(dada_client_t *client, void *buffer, uint64_t bytes, uint64_t block_id);

#endif