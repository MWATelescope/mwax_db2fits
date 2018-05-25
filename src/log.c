#include <stdio.h>
#include <stdarg.h>
#include "utils.h"
#include "log.h"

extern int LOG_LEVEL;

void log_debug(const char* message, ...)
{
    char buffer[2048]="";
    
    va_list args;
    va_start(args, message);
    vsnprintf(buffer, sizeof(buffer), message, args);    
    va_end(args);

    logx(XC_LOG_DEBUG, "(DEBUG)", buffer);
}

void log_info(const char* message, ...)
{
    char buffer[2048]="";
    
    va_list args;
    va_start(args, message);
    vsnprintf(buffer, sizeof(buffer), message, args);    
    va_end(args);

    logx(XC_LOG_INFO, "(INFO)", buffer);
}

void log_warning(const char* message, ...)
{
    char buffer[2048]="";
    
    va_list args;
    va_start(args, message);
    vsnprintf(buffer, sizeof(buffer), message, args);    
    va_end(args);

    logx(XC_LOG_WARNING, "(WARNING)", buffer);
}

void log_error(const char* message, ...)
{
    char buffer[2048]="";
    
    va_list args;
    va_start(args, message);
    vsnprintf(buffer, sizeof(buffer), message, args);    
    va_end(args);

    logx(XC_LOG_ERROR, "(ERROR)", buffer);
}

void logx(int level, const char* levelstr, const char* message)
{
    if (LOG_LEVEL>=level)
    {                
        char timestring[20] = "";

        get_time_string_for_log(timestring);
        
        fprintf(stdout, ">%s, %s, %s\n", timestring, levelstr, message);
    }
}