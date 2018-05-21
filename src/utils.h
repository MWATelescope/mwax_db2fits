/*
 * utils.h
 *
 *  Created on: 21-May-2018
 *      Author: Greg Sleap
 */
#ifndef XC_UTILS_H_
#define XC_UTILS_H_
#include <time.h>

int get_time_struct(struct tm **out_timeinfo);
int get_time_string_for_fits(char *timestring);
int get_time_string_for_log(char *timestring);

#endif