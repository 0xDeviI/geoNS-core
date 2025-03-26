#include "io.h"


uchar is_io_initialized = 0;
uchar cwd[MAX_SYS_PATH_LENGTH];


uchar *exec(uchar *format, ...) {
    uchar* result = (uchar *) memalloc(MAX_SYS_OUTPUT_LENGTH);
    if (result == NULL) {
        perror("Memory error");
        return NULL;
    }

    va_list args;
    va_start(args, format);
    char command[MAX_SYS_COMMAND_LENGTH_PER_EXEC];
    vsnprintf(command, sizeof(command), format, args);
    va_end(args);


    FILE* pipe = popen(command, "r");
    if (pipe == NULL) {
        perror("popen");
        free(result);
        return NULL;
    }

    char buffer[MAX_SYS_OUTPUT_CHUNK_SIZE];
    while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
        strcat(result, buffer);
    }

    if (pclose(pipe) == -1) {
        perror("pclose");
        free(result);
        return NULL;
    }

    return result;
}


uchar is_absolute_path(uchar *path) {
    if (path == NULL) {
        return 0;
    }
    
    return path[0] == '/';
}


char is_directory_path(uchar *path) {
    struct stat path_stat;

    if (stat(path, &path_stat) != 0) {
        return -1;
    }

    if (S_ISDIR(path_stat.st_mode)) {
        return 1;
    }

    return 0;
}


uchar is_file_exist(uchar *file_path) {
    // Should only check for files, not directories
    struct stat path_stat;
    return stat(file_path, &path_stat) == 0 && S_ISREG(path_stat.st_mode);
}


ulong get_file_size(uchar *file_path) {
    FILE *fp = fopen(file_path, "rb");
    
    if (fp == NULL)
        return 0;

    if (fseek(fp, 0, SEEK_END) < 0) {
        fclose(fp);
        return 0;
    }

    long file_size = ftell(fp);
    fclose(fp);
    return file_size;
}


void get_cwd_path(uchar *file_path, ushort size_of_file_path) {
    strncpy(file_path, cwd, size_of_file_path - 1);
    file_path[strlen(cwd)] = '\0';
}


uchar is_directory_exists(uchar *directory) {
    struct stat sb;
    return stat(directory, &sb) == 0 && S_ISDIR(sb.st_mode);
}


uchar is_sproc_exists(void) {
    return is_directory_exists("/proc");
}


void init_io_system(const char *exec_path) {
    if (is_sproc_exists()) {
        #if OS_LINUX == 1
        readlink("/proc/self/exe", cwd, MAX_SYS_PATH_LENGTH);
        #elif OS_FREE_BSD == 1
        readlink("/proc/curproc/file", cwd, MAX_SYS_PATH_LENGTH);
        #elif OS_SOLARIS == 1
        readlink("/proc/self/path/a.out", cwd, MAX_SYS_PATH_LENGTH);
        #endif
    }
    else {
        if (exec_path[0] == '/') {
            strncpy(cwd, exec_path, MAX_SYS_PATH_LENGTH);
        }
        else if (strchr(exec_path, '/') != NULL) {
            #if OS_WINDOWS == 1
            // TODO: coding windows version of path handling
            #else
            if (getcwd(cwd, sizeof(cwd)) != NULL) {
                strncat(cwd, exec_path + 1, MAX_SYS_PATH_LENGTH - 1);
            }
            #endif
        }
    }
    
    for (ushort i = strlen(cwd) - 1; i >= 0; i--)
        if (cwd[i] == '/')
            break;
        else
            cwd[i] = '\0';
    is_io_initialized = 1;
}


uchar get_directory_entries(char *file_path, char *uri, char **body, size_t *body_size) {
    DIR *dir;
    struct dirent *entry;

    dir = opendir(file_path);
    if (dir == NULL) {
        perror("Directory error");
        return 0;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        size_t entry_length = strlen(entry->d_name) + strlen(uri) + 64;
        char *new_body = realloc(*body, *body_size + entry_length + 1);
        if (new_body == NULL) {
            perror("Memory error");
            free(*body);
            *body = NULL;
            *body_size = 0;
            closedir(dir);
            return 0;
        }
        *body = new_body;

        snprintf(*body + *body_size, entry_length + 1,
                 "<a href=\"%s%s%s\">%s</a><br>\n",
                 uri,
                 (uri[strlen(uri) - 1] == '/') ? "" : "/",
                 entry->d_name,
                 entry->d_name);

        *body_size += strlen(*body + *body_size);
    }
    closedir(dir);
    return 1;
}