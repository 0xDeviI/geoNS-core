#ifndef GEONS_LOGGER_H
#define GEONS_LOGGER_H 1

#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdarg.h>
#include "../io/io.h"
#include "../core/core.h"

typedef enum eLogType {
    DEBUG,
    INFO,
    WARNING,
    ERROR,
    FATAL
} LogType;

extern uchar log_file_name[MAX_LOG_FILE_NAME_LENGTH];

void init_logger(void);
char msglog(LogType type, uchar *message, ...);

#endif // !GEONS_LOGGER_H