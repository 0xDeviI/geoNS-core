#ifndef GEONS_CORE_H
#define GEONS_CORE_H 1

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define GEONS_EXEC_NAME "geoNS-core"
#define GEONS_MAJOR_VERSION 0
#define GEONS_MINOR_VERSION 1
#define GEONS_PATCH_VERSION 0
#define GEONS_VERSION TOSTRING(GEONS_MAJOR_VERSION) "." TOSTRING(GEONS_MINOR_VERSION) "." TOSTRING(GEONS_PATCH_VERSION)
#define COMPILE_TIME __DATE__ ", " __TIME__

#include <stdio.h>
#include <stdlib.h>
#include "constants.h"
#include "globals.h"
#include "types.h"

extern uchar is_debugging;
extern uchar is_geons_running;

void *memalloc(size_t size);

#endif // !GEONS_CORE_H