#ifndef MAP_H
#define MAP_H

#include "list.h"




typedef struct Pair {
    void* key;
    void* value;
} MapPair;

typedef struct Map {
    int (*lower_than)(void *key1, void *key2);
    int (*is_equal)(void *key1, void *key2);
    List *ls;
} Map;


Map *map_create(int (*is_equal)(void *key1, void *key2));
Map *sorted_map_create(int (*lower_than)(void*, void*), int (*is_equal)(void*, void*));
void map_insert(Map *map, void *key, void *value);
MapPair *map_remove(Map *map, void *key);
MapPair *map_search(Map *map, void *key);
MapPair *map_first(Map *map);
MapPair *map_next(Map *map);
void map_clean(Map *map);
void map_set_equal(Map* map, int (*is_equal)(void*, void*));
int map_size(Map* map);



#endif /* MAP_H */
