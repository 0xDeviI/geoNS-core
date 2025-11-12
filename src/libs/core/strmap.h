#ifndef GEONS_STRING_MAP_H
#define GEONS_STRING_MAP_H 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../core/core.h"

typedef struct MapNode {
    char *key;
    char *value;
    struct MapNode *next;
} MapNode;

typedef struct {
    MapNode *head;
    int size;
} StringMap;

StringMap* create_string_map();
void string_map_put(StringMap *map, uchar *key, uchar *value);
uchar* string_map_get(StringMap *map, uchar *key);
uchar string_map_contains(StringMap *map, uchar *key);
void string_map_remove(StringMap *map, uchar *key);
void string_map_free(StringMap *map);
void string_map_print(StringMap *map);

#endif // GEONS_STRING_MAP_H