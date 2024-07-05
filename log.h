#ifndef LOG_H
#define LOG_H

void log_trace(const char* message, ...);
void log_info(const char* message, ...);
void log_warn(const char* message, ...);
void log_error(const char* message, ...);
void log_none(const char* message, ...);

#ifdef NDEBUG
#define log_debug(const char* message, ...) (void)0
#else
void log_debug(const char* message, ...);
#endif

#endif