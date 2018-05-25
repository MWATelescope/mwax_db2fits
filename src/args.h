/*
 * args.h
 *
 *  Created on: 21-May-2018
 *      Author: Greg Sleap
 */
#ifndef XC_ARGS_H_
#define XC_ARGS_H_

#include "dada.h"

// Command line Args
struct globalArgs_t
{
    key_t input_db_key;
    int listen_port;
    char* listen_interface;
    char* destination_url;
    char* metafits_path;
};

typedef struct globalArgs_t globalArgs_t;

extern globalArgs_t globalArgs;

void print_usage(void);
int process_args(int argc, char* argv[]);

#endif