/**
 * @file utils.c
 * @author Greg Sleap
 * @date 21 May 2018
 * @brief This is the code that handles misc functions like time and formatting
 *
 */
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

int get_time_struct(struct tm **out_timeinfo)
{
    time_t rawtime;
    
    time(&rawtime);
    *out_timeinfo = localtime(&rawtime);
        
    int month = (*out_timeinfo)->tm_mon + 1; 
    int year = (*out_timeinfo)->tm_year + 1900;

    (*out_timeinfo)->tm_mon = month;
    (*out_timeinfo)->tm_year = year;

    return EXIT_SUCCESS;
}

int get_time_string_for_fits(char *timestring)
{
    struct tm *timeinfo;    
    get_time_struct(&timeinfo);

    sprintf(timestring, "%4d%02d%02d%02d%02d%02d", timeinfo->tm_year, timeinfo->tm_mon, timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);

    return EXIT_SUCCESS;
}

int get_time_string_for_log(char *timestring)
{
    struct tm *timeinfo;    
    get_time_struct(&timeinfo);
    
    sprintf(timestring, "%4d-%02d-%02d %02d:%02d:%02d", timeinfo->tm_year, timeinfo->tm_mon, timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);

    return EXIT_SUCCESS;
}