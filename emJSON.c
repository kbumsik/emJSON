#include "emJSON.h"
#include <string.h>

json_t emJSON_init()
{
    void *buffer = malloc(EMJSON_INIT_BUF_SIZE);
    json_entry_t *table = (json_entry_t *)calloc(EMJSON_INIT_TABLE_SIZE, sizeof(json_entry_t));
    json_t result = json_init(buffer, EMJSON_INIT_BUF_SIZE, table, EMJSON_INIT_TABLE_SIZE);
    return result;
}

int emJSON_delete(json_t *obj, char *key)
{
    return json_delete(obj, key);
}

int emJSON_clear(json_t *obj)
{
    return json_clear(obj);
}

/*******************************************************************************
 * Insertion functions
 ******************************************************************************/

int emJSON_insert(json_t *obj, char *key, void *value, json_value_e type)
{
    int ret;
    ret = json_insert(obj, key, value, type);
    // TODO: Increase size then entry_num/size = 3/4?
    while (ret != JSON_OK)
    {
        json_entry_t *new_table;
        json_entry_t *old_table;
        int table_size = obj->entry_size;
        
        void *new_buf;
        void *old_buf;
        int buf_size = obj->buf_size;
        
        switch (ret)
        {
        case JSON_KEY_EXISTS:
            return JSON_KEY_EXISTS;
        case JSON_TABLE_FULL:
            old_table = obj->entry_table;
            table_size *= 2;
            new_table = calloc(table_size, sizeof(json_entry_t));
            json_replace_table(obj, new_table, table_size);
            free(old_table);
            ret = json_insert(obj, key, value, type);
            break;
        case JSON_BUFFER_FULL:
            old_buf = obj->buf;
            buf_size += 32;
            new_buf = malloc(buf_size);
            json_replace_buffer(obj, new_buf, buf_size);
            free(old_buf);
            ret = json_insert(obj, key, value, type);
            break;
        default:
            return JSON_ERROR;
        }
    }
    return JSON_OK;
}

int emJSON_insert_str(json_t *obj, char *key, char *value)
{
    return emJSON_insert(obj, key, value, JSON_STRING);
}

int emJSON_insert_int(json_t *obj, char *key, int value)
{
    return emJSON_insert(obj, key, &value, JSON_INT);
}

int emJSON_insert_float(json_t *obj, char *key, float value)
{
    return emJSON_insert(obj, key, &value, JSON_FLOAT);
}

/*******************************************************************************
 * Getter functions
 ******************************************************************************/

void *emJSON_get(json_t *obj, char *key)
{
    return json_get(obj, key);
}

char *emJSON_get_str(json_t *obj, char *key)
{
    return json_get_str(obj, key);
}

int emJSON_get_int(json_t *obj, char *key)
{
    return json_get_int(obj, key);
}

float emJSON_get_float(json_t *obj, char *key)
{
    return json_get_float(obj, key);
}

/*******************************************************************************
 * String-related functions
 ******************************************************************************/

char *emJSON_string(json_t *obj)
{
    int len = json_strlen(obj) + 1;
    char *str = malloc(len * sizeof(char));
    json_strcpy(str, obj);
    return str;
}

int emJSON_strcpy(char *dest, json_t *obj)
{
    return json_strcpy(dest, obj);
}

int emJSON_free(json_t *obj)
{
    // free table
    free(obj->entry_table);
    // free buffer
    free(obj->buf);
    // Clear
    memset(obj, 0, sizeof(json_t));
    return 0;
}
