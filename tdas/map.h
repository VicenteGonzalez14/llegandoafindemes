#ifndef MAP_H
#define MAP_H
#include "list.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct Pair {
    void* key;
    void* value;
} Pair;



typedef struct Map Map;

Map *map_create(int (*is_equal)(void *key1, void *key2)); // unsorted map

Map *sorted_map_create(int (*lower_than)(void *key1, void *key2));

void map_insert(Map *map, void *key, void *value);

Map *map_remove(Map *map, void *key);

Map *map_search(Map *map, void *key);

Map *map_first(Map *map);

Map *map_next(Map *map);

void map_clean(Map *map);

#endif 