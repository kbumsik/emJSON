/**
 * This library is inspired by Python Dictionary implementation.
 * Look string_hash() function in https://svn.python.org/projects/python/trunk/Objects/stringobject.c
 * Also see http://svn.python.org/projects/python/trunk/Objects/dictobject.c for open addressing.
 */
#ifndef __EMJSON_H__
#define __EMJSON_H__

#include "json.h"

// Settings
#define EMJSON_INIT_BUF_SIZE    64
#define EMJSON_INIT_TABLE_SIZE  8

// high-level functions
json_t emJSON_init();
int emJSON_insert(json_t *obj, char *key, void *value);
void *emJSON_get(json_t *obj, char *key);
// int emJSON_set(json_t *obj, char *key, void *value); // TODO: Implement json_set()
int emJSON_delete(json_t *obj, char *key);
int emJSON_clear(json_t *obj);

char *emJSON_string(json_t *obj);

int emJSON_free(json_t *obj);

#endif	// __EMJSON_H__
