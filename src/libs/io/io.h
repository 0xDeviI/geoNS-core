#ifndef GEONS_IO_H
#define GEONS_IO_H 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../core/globals.h"
#ifdef OS_LINUX
    #include <unistd.h>
    #include <fcntl.h>
#elif defined(OS_WINDOWS)
    #include <windows.h>
    #include <io.h>
    #define F_OK 0
    #define access _access
#endif

#include <sys/stat.h>
#include "../core/types.h"
#include "../core/constants.h"


extern uchar cwd[MAX_SYS_PATH_LENGTH];

uchar is_file_exist(uchar *file_path);
ulong get_file_size(uchar *file_path);
void get_cwd_path(uchar *file_path, ushort size_of_file_path);
uchar is_directory_exists(uchar *directory);
uchar is_sproc_exists(void);
void init_io_system(const char *exec_path);

#endif // !GEONS_IO_H