/**
 * @file global.h
 * @author Greg Sleap
 * @date 5 Jul 2018
 * @brief This is the header for anything global 
 *
 */
#ifndef XC_GLOBAL_H_
#define XC_GLOBAL_H_

#include <pthread.h>
#include "dada.h"

int initialise_quit();
int set_quit(int value);
int get_quit();
int destroy_quit();

#endif