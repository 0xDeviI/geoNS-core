#include "strmap.h"

StringMap* create_string_map() {
    StringMap *map = malloc(sizeof(StringMap));
    map->head = NULL;
    map->size = 0;
    return map;
}

void string_map_put(StringMap *map, const char *key, const char *value) {
    // Check if key already exists
    MapNode *current = map->head;
    while (current != NULL) {
        if (strcmp(current->key, key) == 0) {
            free(current->value);
            current->value = strdup(value);
            return;
        }
        current = current->next;
    }
    
    // Create new node
    MapNode *new_node = malloc(sizeof(MapNode));
    new_node->key = strdup(key);
    new_node->value = strdup(value);
    new_node->next = map->head;
    map->head = new_node;
    map->size++;
}

char* string_map_get(StringMap *map, const char *key) {
    MapNode *current = map->head;
    while (current != NULL) {
        if (strcmp(current->key, key) == 0) {
            return current->value;
        }
        current = current->next;
    }
    return NULL;
}

int string_map_contains(StringMap *map, const char *key) {
    return string_map_get(map, key) != NULL;
}

void string_map_remove(StringMap *map, const char *key) {
    MapNode *current = map->head;
    MapNode *prev = NULL;
    
    while (current != NULL) {
        if (strcmp(current->key, key) == 0) {
            if (prev == NULL) {
                map->head = current->next;
            } else {
                prev->next = current->next;
            }
            free(current->key);
            free(current->value);
            free(current);
            map->size--;
            return;
        }
        prev = current;
        current = current->next;
    }
}

void string_map_free(StringMap *map) {
    MapNode *current = map->head;
    while (current != NULL) {
        MapNode *next = current->next;
        free(current->key);
        free(current->value);
        free(current);
        current = next;
    }
    free(map);
}

void string_map_print(StringMap *map) {
    MapNode *current = map->head;
    printf("Map contents (%d items):\n", map->size);
    while (current != NULL) {
        printf("  %s: %s\n", current->key, current->value);
        current = current->next;
    }
}