/**
 * @file args.h
 * @author Greg Sleap
 * @date 21 May 2018
 * @brief This is the header for the code that parses and validates command line arguments
 *
 */
#pragma once

#include <sys/ipc.h> // for key_t

// Command line Args
typedef struct
{
    key_t input_db_key;    
    char* destination_path;
    char* metafits_path;
    char* health_ip;
    char* stats_path;
    int health_port;  
    long file_size_limit;  
} globalArgs_s;

void print_usage();
int process_args(int argc, char* argv[], globalArgs_s* globalArgs);