/*
 * log.h
 *
 *  Created on: 18-May-2018
 *      Author: Greg Sleap
 */
#ifndef XC_LOG_H_
#define XC_LOG_H_

#define XC_LOG_ERROR 0
#define XC_LOG_WARNING 1 
#define XC_LOG_INFO 2 
#define XC_LOG_DEBUG 3

void logx(int level, const char* levelstr, const char* message);
void log_debug(const char* message, ...);
void log_info(const char* message, ...);
void log_warning(const char* message, ...);
void log_error(const char* message, ...);

#endif /* XC_LOG_H_ */