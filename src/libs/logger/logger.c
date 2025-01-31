#include "logger.h"


uchar log_file_name[MAX_LOG_FILE_NAME_LENGTH];


void init_logger(void) {
    if (!is_io_initialized)
        return;

    time_t timer = time(NULL);
    struct tm *tm_info = localtime(&timer);
    strftime(log_file_name, MAX_LOG_FILE_NAME_LENGTH, "%Y-%m-%d_%H-%M-%S.log", tm_info);
}


char msglog(LogType type, uchar *message, ...) {
    if (!is_io_initialized)
        return 0;
    
    if ((type == DEBUG && is_debugging) || (type != DEBUG)) {
        uchar log_file_path[MAX_SYS_PATH_LENGTH];
        get_cwd_path(log_file_path, sizeof(log_file_path));
        strncat(log_file_path, "logs/", sizeof(log_file_path) - 1);

        if (!is_directory_exists(log_file_path)) {
            if (mkdir(log_file_path, 0700)) {
                perror("Directory creation error");
                return 0;
            }
        }


        va_list args;
        va_start(args, message);
        uchar buffer[MAX_LOG_LENGTH];
        vsnprintf(buffer, MAX_LOG_LENGTH, message, args);
        va_end(args);

        strncat(log_file_path, log_file_name, sizeof(log_file_path) - strlen(log_file_path) - 1);
        FILE *log_file = fopen(log_file_path, "a");
        
        if (log_file == NULL) {
            perror("Log file error");
            return 0;
        }

        // Calculate necessary log size
        size_t size_of_message = strlen(buffer);
        size_t log_size = MAX_LOG_PRE_TEXT_LENGTH + size_of_message + 50; // Extra space for timestamp and log level
        uchar *log_data = (uchar *) memalloc(log_size);
        if (log_data == NULL) {
            fclose(log_file);
            perror("Memory error");
            return 0;
        }

        uchar log_char = 'U';
        time_t timer = time(NULL);
        struct tm *tm_info = localtime(&timer);
        uchar current_time[MAX_LOG_TIME_LENGTH];
        strftime(current_time, sizeof(current_time), "%Y/%m/%d - %H:%M:%S", tm_info);

        switch (type) {
            case DEBUG: log_char = 'D'; break;
            case INFO: log_char = 'I'; break;
            case WARNING: log_char = 'W'; break;
            case ERROR: log_char = 'E'; break;
            case FATAL: log_char = 'F'; break;
            default: log_char = 'U'; break; // Unknown type
        }


        if (size_of_message > 0 && buffer[size_of_message - 1] == '\n') {
            size_of_message--; // Decrease the size to exclude the newline
        }
        // Properly format the log message
        snprintf(log_data, log_size, "%c\t[%s]\t%.*s\n", log_char, current_time, (int)size_of_message, buffer);

        // Write to file
        fwrite(log_data, strlen(log_data), 1, log_file);

        fclose(log_file);
        free(log_data);
    }
}