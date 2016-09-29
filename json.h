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

typedef enum
{
    //integer,
    //number,
    string
    //object,
    //array,
    //boolean,
    //null_value
}json_value_e;

typedef struct
{
    char *key;
    int32_t hash;
    size_t value_size;
    void *value_ptr;
    //json_value_e value_type;
}json_entry_t;

typedef struct
{
    int buf_size;
    void    *buf;
    size_t  buf_idx;
    json_entry_t *entry_table;
    size_t  entry_size;
    int entry_count;
}json_t;

// core utility functions
int32_t json_hash(char *str);

// lower-level basic functions
json_t json_init(void *buffer, size_t buf_size, json_entry_t table[], size_t table_size);
int json_insert(json_t *obj, char *key, void *value);
void *json_get(json_t *obj, char *key);
// int json_set(json_t *obj, char *key, void *value); // TODO: Implement json_set()
int json_delete(json_t *obj, char *key);
int json_clear(json_t *obj);

// String-related functions
int json_strcpy(char *dest, json_t *obj);
int json_strlen(json_t *obj);

// Buffer and memory management functions
int json_replace_buffer(json_t *obj, void *new_buf, size_t size);
int json_replace_table(json_t *obj, json_entry_t new_table[], size_t size);

#endif  // __JSON_H__
