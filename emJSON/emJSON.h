#ifndef __EMJSON_H__
#define __EMJSON_H__

#include "json.h"

// Settings
#ifndef EMJSON_INIT_BUF_SIZE
    #define EMJSON_INIT_BUF_SIZE    256
#endif
#ifndef EMJSON_INIT_TABLE_SIZE
    #define EMJSON_INIT_TABLE_SIZE  4
#endif

#ifdef __cplusplus
extern "C"{
#endif

// high-level functions
json_t emJSON_init(void);
int emJSON_parse(json_t *obj, char *input);
int emJSON_delete(json_t *obj, char *key);
int emJSON_clear(json_t *obj);

// Insertion functions
int emJSON_insert(json_t *obj, char *key, void *value, json_type_t type);
int emJSON_insert_str(json_t *obj, char *key, char *value);
int emJSON_insert_int(json_t *obj, char *key, int value);
int emJSON_insert_float(json_t *obj, char *key, float value);

// Getter functions
void *emJSON_get(json_t *obj, char *key, json_type_t type);
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

#ifdef __cplusplus
}
#endif

#endif    // __EMJSON_H__

