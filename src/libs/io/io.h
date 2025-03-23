#ifndef GEONS_IO_H
#define GEONS_IO_H 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../core/globals.h"
#ifdef OS_LINUX
    #include <unistd.h>
    #include <fcntl.h>
    #include <dirent.h>
#elif defined(OS_WINDOWS)
    #include <windows.h>
    #include <io.h>
    #define F_OK 0
    #define access _access
#endif

#include <sys/stat.h>
#include "../core/core.h"


extern uchar is_io_initialized;
extern uchar cwd[MAX_SYS_PATH_LENGTH];

uchar* exec(uchar* format, ...);
char is_directory_path(uchar *path);
uchar is_absolute_path(uchar *path);
uchar is_file_exist(uchar *file_path);
ulong get_file_size(uchar *file_path);
void get_cwd_path(uchar *file_path, ushort size_of_file_path);
uchar is_directory_exists(uchar *directory);
uchar is_sproc_exists(void);
void init_io_system(const char *exec_path);
uchar get_directory_entries(char *file_path, char *uri, char **body, size_t *body_size);

#endif // !GEONS_IO_H