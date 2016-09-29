#ifndef __JSON_H__
#define __JSON_H__

#include <stdint.h>
#include <stdlib.h>

#define JSON_OK             0
#define JSON_ERROR          -1   // Unknown error
#define JSON_NO_MATCHED_KEY	-2
#define JSON_KEY_EXISTS	    -3
#define JSON_TABLE_FULL	    -4
#define JSON_BUFFER_FULL    -5
#define JSON_ENTRY_BUFFER_FULL    -6

typedef enum
{
    JSON_INT,
    JSON_FLOAT,
    JSON_STRING
    //object,
    //array,
    //boolean,
    //null_value
}json_value_e;

typedef struct
{
    json_value_e value_type;
    int32_t hash;
    void *value_ptr;
    char *key;
    size_t value_size;
}json_entry_t;

typedef struct
{
    int buf_size;
    size_t  buf_idx;
    size_t  table_size;
    int entry_count;
}json_header_t;

typedef struct
{
    void *buf;
    json_header_t *header;
    json_entry_t *entry_table;
    void *content;
}json_t;

// core utility functions
int32_t json_hash(char *str);

// lower-level basic functions
json_t json_init(void *buffer, size_t buf_size, size_t table_size);
int json_delete(json_t *obj, char *key);
int json_clear(json_t *obj);

// Insertion functions
int json_insert(json_t *obj, char *key, char *value, json_value_e type);
int json_insert_str(json_t *obj, char *key, char *value);
int json_insert_int(json_t *obj, char *key, int32_t value);
int json_insert_float(json_t *obj, char *key, float value);

// Getter functions
void *json_get(json_t *obj, char *key);
char *json_get_str(json_t *obj, char *key);
int json_get_int(json_t *obj, char *key);
float json_get_float(json_t *obj, char *key);

// Setter functions
int json_set(json_t *obj, char *key, void *value);
int json_set_str(json_t *obj, char *key, char *value);
int json_set_int(json_t *obj, char *key, int value);
int json_set_float(json_t *obj, char *key, float value);

// String-related functions
int json_strcpy(char *dest, json_t *obj);
int json_strlen(json_t *obj);

// Buffer and memory management functions
int json_replace_buffer(json_t *obj, void *new_buf, size_t size);
int json_double_table(json_t *obj);
json_t json_copy(void *dest_buf, json_t *obj);

#endif  // __JSON_H__
