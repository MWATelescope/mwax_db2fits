/*
 * dada.h
 *
 *  Created on: 23-May-2018
 *      Author: Greg Sleap
 */
#ifndef XC_DADA_H_
#define XC_DADA_H_

#include <sys/types.h> 

#define HAVE_HWLOC      // This is used by the psrdada library to set if we have the hwloc library or not. This lib is used to abstract NUMA / architecture.
typedef __key_t key_t;  // This is needed because my types.h is not pointing at the correct one.

#endif