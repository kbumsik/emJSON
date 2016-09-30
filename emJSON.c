#include "emJSON.h"
#include <string.h>

json_t emJSON_init()
{
    void *buffer = malloc(EMJSON_INIT_BUF_SIZE);
    json_t result = json_init(buffer, EMJSON_INIT_BUF_SIZE, EMJSON_INIT_TABLE_SIZE);
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

int emJSON_insert(json_t *obj, char *key, void *value, json_value_t type)
{
    int ret;
    ret = json_insert(obj, key, value, type);
    // TODO: Increase size then entry_num/size = 3/4?
    while (ret != JSON_OK)
    {
        int table_size = obj->header->table_size;
        
        void *new_buf;
        void *old_buf;
        int buf_size = obj->header->buf_size;
        
        switch (ret)
        {
        case JSON_KEY_EXISTS:
            return JSON_KEY_EXISTS;
        case JSON_TABLE_FULL:
            ret = json_double_table(obj);
            if (JSON_OK == ret)
            {
                ret = json_insert(obj, key, value, type);
            }
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
 * Setter functions
 ******************************************************************************/

int emJSON_set(json_t *obj, char *key, void *value)
{
    return json_set(obj, key, value);
}

int emJSON_set_str(json_t *obj, char *key, char *value)
{
    int ret;
    ret = json_set_str(obj, key, value);
    // TODO: Increase size then entry_num/size = 3/4?
    if (ret == JSON_ENTRY_BUFFER_FULL)
    {
        emJSON_delete(obj, key);
        ret = emJSON_insert_str(obj, key, value);
    }
    return ret;
}

int emJSON_set_int(json_t *obj, char *key, int value)
{
    return json_set_int(obj, key, value);
}

int emJSON_set_float(json_t *obj, char *key, float value)
{
    return json_set_float(obj, key, value);
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
