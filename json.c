#include "json.h"
#include <string.h>

#define PERTURB_SHIFT 5

int32_t json_hash(char *str)
{
    char *ptr = str;
    int len = 0;
    int32_t result = *ptr << 7;
    for (; *ptr != '\0'; ptr++)
    {
        char cha = *ptr;
        result = (1000003 * result) ^ cha;
        len += 1;
    }
    result ^= len;
    return result;	
}

json_t json_init(void *buffer, size_t buf_size, json_entry_t table[], size_t table_size)
{
    json_t result = { 
        .buf = buffer,
        .buf_size = buf_size,
        .buf_idx = 0,
        .entry_table = table,
        .entry_size = table_size,
        .entry_count = 0
    };
    // Clean buffer
    memset(buffer, 0, buf_size);
    memset(table, 0, table_size * sizeof(json_entry_t));
    return result;
}

int json_insert(json_t *obj, char *key, void *value)
{
    if (obj->entry_count >= obj->entry_size)
    {
        return JSON_TABLE_FULL;
    }
    
    // buffer size check
    int buf_required = (strlen(key) + 1) + (strlen(value) + 1);
    if (obj->buf_idx + buf_required > obj->buf_size)
    {
        return JSON_BUFFER_FULL;
    }
    
    // construct entry object first, with hash.
    json_entry_t new_entry = {};
    new_entry.hash = json_hash(key);
    
    // put into the table
    int32_t new_idx = new_entry.hash & (obj->entry_size - 1);
    
    int perturb = new_entry.hash;
    while (NULL != (obj->entry_table[new_idx].key))
    {	// collision, open addressing
        if (new_entry.hash == obj->entry_table[new_idx].hash)
        {	// collision, and the hash are the same (same key)
            return JSON_KEY_EXISTS;
        }
        new_idx = (5 * new_idx) + 1 + perturb;
        perturb >>= PERTURB_SHIFT;
        new_idx = new_idx & (obj->entry_size - 1);
    }
    
    // then put key into the buffer
    void *key_ptr = obj->buf + obj->buf_idx;
    strcpy((char *)key_ptr, key);
    new_entry.key = key_ptr;
    obj->buf_idx += strlen(key_ptr) + 1;
    
    // put value into the buffer
    void *value_ptr = obj->buf + obj->buf_idx;
    strcpy((char *)value_ptr, (char *)value);
    new_entry.value_ptr = value_ptr;
    obj->buf_idx += strlen(value_ptr) + 1;
    
    // Put the new entry
    obj->entry_table[new_idx] = new_entry;
    
    obj->entry_count += 1;
    return JSON_OK;
}

void *json_get(json_t *obj, char *key)
{
    void *result;
    int32_t hash = json_hash(key);
    int idx = hash & (obj->entry_size - 1);
    json_entry_t entry = obj->entry_table[idx];
    
    // variables for open addressing
    int perturb = hash;
    uint8_t checked[obj->entry_size];
    int count = 0;
    
    if (hash == entry.hash)
    {   // Good
        goto end_success;
    }
    
    // On collision
    memset(checked, 0, obj->entry_size);
    while (hash != entry.hash)
    {
        if (0 == checked[idx])
        {
            count++;
            if (count >= obj->entry_size)
            {   // When the table is full and visited all entries
                goto end_error;
            }
        }
        if (NULL == entry.hash)
        {   // When we get empty slot
            goto end_error;
        }
        // open addressing
        idx = (5 * idx) + 1 + perturb;
        perturb >>= PERTURB_SHIFT;
        idx = idx & (obj->entry_size - 1);
        // refresh
        entry = obj->entry_table[idx];
    }
end_success:
    result = entry.value_ptr;
    return result;
end_error:
    return NULL;
}

int json_strcpy(char *dest, json_t *obj)
{
    int idx = 0;
    memset(dest + idx, '{', 1);
    idx += 1;
    // start
    for (int i = 0; i < obj->entry_size; i++)
    {
        json_entry_t entry = obj->entry_table[i];
        if (NULL == entry.key)
        {	// empty entry
            continue;
        }
        // copy key: "<key>":
        memset(dest + idx, '\"', 1);
        idx += 1;
        int str_len = strlen(entry.key);
        strcpy(dest + idx, entry.key);
        strcpy(dest + idx + str_len, "\":");
        idx += str_len + 2;
        // copy value: "<value>",
        memset(dest + idx, '\"', 1);
        idx += 1;
        str_len = strlen(entry.value_ptr);
        strcpy(dest + idx, entry.value_ptr);
        strcpy(dest + idx + str_len, "\",");
        idx += str_len + 2;
    }
    // it's the end
    // -1 is to remove the last ','.
    strcpy(dest + idx -1, "}");
    return JSON_OK;
}

int json_strlen(json_t *obj)
{
    int idx = 0;
    idx += 1;	// '{'
    // start
    for (int i = 0; i < obj->entry_size; i++)
    {
        json_entry_t entry = obj->entry_table[i];
        if (NULL == entry.key)
        {	// empty entry
            continue;
        }
        // "<key>":
        idx += 1; // '\"'
        int str_len = strlen(entry.key);
        idx += str_len + 2;	// <key>":
        // "<value>",
        idx += 1;	// '\"'
        str_len = strlen(entry.value_ptr);
        idx += str_len + 2; // <value>",
    }
    // it's the end, '}'
    // consider removing the last ','.
    return idx;
}

int json_replace_buffer(json_t *obj, void *new_buf, size_t size)
{
    void *old_buf = obj->buf;
    int offset = new_buf - old_buf;
    
    memcpy(new_buf, old_buf, obj->buf_idx);
    
    // move key and value pointers
    for (int i = 0; i < obj->entry_size; i++)
    {
        json_entry_t *entry = (obj->entry_table + i);
        if (NULL == entry->key)
        {
            continue;
        }
        entry->key += offset;
        entry->value_ptr += offset;
    }
    
    // finally change the buffer
    obj->buf = new_buf;
    obj->buf_size = size;
    return JSON_OK;
}


int json_replace_table(json_t *obj, json_entry_t new_table[], size_t size)
{
    // create a temp JSON object
    uint8_t tmp_buf[obj->buf_idx];
    json_t tmp_obj = json_init(tmp_buf, obj->buf_idx, new_table, size);
    
    for (int i = 0; i < obj->entry_size; i++)
    {
        json_entry_t *entry = (obj->entry_table + i);
        if (NULL == entry->key)
        {
            continue;
        }
        json_insert(&tmp_obj, entry->key, entry->value_ptr);
    }
    // Then replace buffer
    memset(obj->buf, 0, obj->buf_idx);
    json_replace_buffer(&tmp_obj, obj->buf, obj->buf_size);
    
    // Then replace table
    obj->entry_table = new_table;
    obj->entry_size = size;
    return JSON_OK;
}