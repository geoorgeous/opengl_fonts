#include <stdarg.h>
#include <stdio.h>

#include "log.h"

#define LOG_TAG_TRACE "TRACE | "
#define LOG_TAG_INFO  "INFO  | "
#define LOG_TAG_WARN  "WARN  | "
#define LOG_TAG_ERROR "ERROR | "
#define LOG_TAG_DEBUG "DEBUG | "

#define FILE_LINE_STR __FILE__":"__LINE__

static void log_printf(FILE* stream, const char* message, va_list args) {
    vfprintf(stream, message, args);
}

void log_trace(const char* message, ...) {
    fprintf(stdout, LOG_TAG_TRACE);
    va_list args;
    va_start(args, message);
    log_printf(stdout, message, args);
    va_end(args);
}

void log_info(const char* message, ...) {
    fprintf(stdout, LOG_TAG_INFO);
    va_list args;
    va_start(args, message);
    log_printf(stdout, message, args);
    va_end(args);
}

void log_warn(const char* message, ...) {
    fprintf(stdout, LOG_TAG_WARN);
    va_list args;
    va_start(args, message);
    log_printf(stdout, message, args);
    va_end(args);
}

void log_error(const char* message, ...) {
    fprintf(stdout, LOG_TAG_ERROR);
    va_list args;
    va_start(args, message);
    log_printf(stderr, message, args);
    va_end(args);
}

#ifndef NDEBUG
void log_debug(const char* message, ...) {
    fprintf(stdout, LOG_TAG_DEBUG);
    va_list args;
    va_start(args, message);
    log_printf(stdout, message, args);
    va_end(args);
}
#endif