/**
 * @file utils.h
 * @author Greg Sleap
 * @date 21 May 2018
 * @brief This is the header for the code that handles misc functions like time and formatting
 *
 */
#pragma once

#include <time.h>

int get_time_struct(struct tm **out_timeinfo);
int get_time_string_for_fits(char *timestring);
int get_time_string_for_log(char *timestring);