#include <stdio.h>
#include <time.h>
#include "log.h"

enum LOG_TYPE_ENUM LOG_LEVEL=LOG_DEBUG;

void log_debug(const char* message)
{
    logx(LOG_DEBUG, "(DEBUG)", message);
}

void log_info(const char* message)
{
    logx(LOG_INFO, "(INFO)", message);
}

void log_warning(const char* message)
{
    logx(LOG_WARNING, "(WARNING)", message);
}

void log_error(const char* message)
{
    logx(LOG_ERROR, "(ERROR)", message);
}

void logx(enum LOG_TYPE_ENUM level, const char* levelstr, const char* message)
{
    if (LOG_LEVEL>=level)
    {
        time_t rawtime;
        struct tm *timeinfo;

        time(&rawtime);
        timeinfo = gmtime(&rawtime);
        
        int month = timeinfo->tm_mon + 1; 
        int year = timeinfo->tm_year + 1900;
        
        fprintf(stdout, "%s%d-%02d-%02d %02d:%02d:%02d, %s, %s\n", ">", year, month, timeinfo->tm_mday, 
                timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, levelstr, message);
    }
}