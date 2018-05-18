#ifndef XC_LOG_H_
#define XC_LOG_H_

enum LOG_TYPE_ENUM {
    LOG_ERROR, 
    LOG_WARNING, 
    LOG_INFO, 
    LOG_DEBUG 
};

void logx(enum LOG_TYPE_ENUM level, const char* levelstr, const char* message);
void log_debug(const char* message);
void log_info(const char* message);
void log_warning(const char* message);
void log_error(const char* message);

#endif /* XC_LOG_H_ */