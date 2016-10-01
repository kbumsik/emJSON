/**
 * This library is inspired by Python Dictionary implementation.
 * Look string_hash() function in https://svn.python.org/projects/python/trunk/Objects/stringobject.c
 * Also see http://svn.python.org/projects/python/trunk/Objects/dictobject.c for open addressing.
 */
#ifndef __EMJSON_H__
#define __EMJSON_H__

#include "json.h"

// Settings
#define EMJSON_INIT_BUF_SIZE    128
#define EMJSON_INIT_TABLE_SIZE  2

// high-level functions
json_t emJSON_init();
int emJSON_parse(json_t *obj, char *input);
int emJSON_delete(json_t *obj, char *key);
int emJSON_clear(json_t *obj);

// Insertion functions
int emJSON_insert(json_t *obj, char *key, void *value, json_value_t type);
int emJSON_insert_str(json_t *obj, char *key, char *value);
int emJSON_insert_int(json_t *obj, char *key, int value);
int emJSON_insert_float(json_t *obj, char *key, float value);

// Getter functions
void *emJSON_get(json_t *obj, char *key);
char *emJSON_get_str(json_t *obj, char *key);
int emJSON_get_int(json_t *obj, char *key);
float emJSON_get_float(json_t *obj, char *key);

// Setter functions
int emJSON_set(json_t *obj, char *key, void *value);
int emJSON_set_str(json_t *obj, char *key, char *value);
int emJSON_set_int(json_t *obj, char *key, int value);
int emJSON_set_float(json_t *obj, char *key, float value);

// String-related functions
char *emJSON_string(json_t *obj);
int emJSON_strcpy(char *dest, json_t *obj);

int emJSON_free(json_t *obj);

#endif	// __EMJSON_H__
