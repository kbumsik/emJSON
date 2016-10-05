#include "json.h"
#include <string.h>
#include <stdio.h>

#define PERTURB_SHIFT 5

typedef struct
{
    int32_t hash;
    void *value_ptr;
    char *key;
    json_value_size_t value_size;
    json_value_t value_type;
}entry_t_;

typedef struct
{
    size_t buf_size;
    size_t buf_idx;
    size_t table_size;
    size_t entry_count;
}header_t_;

// Private functions
static int get_idx_(json_t *obj, char *key);
static int insert_(json_t *obj, char *key, void *value, size_t size, json_value_t type);

// pointer macros
#define header_ptr_(obj)  ((header_t_ *)((obj)->buf))
#define table_ptr_(obj)  ((entry_t_ *)((obj)->buf + sizeof(header_t_)))
#define content_ptr_(obj) ((obj)->buf + sizeof(header_t_) + _table_byte_size(obj))

// pointer size functions
#define buf_size_(obj)  (header_ptr_(obj)->buf_size)

#define buf_idx_(obj)   (header_ptr_(obj)->buf_idx)

#define table_size_(obj)    (header_ptr_(obj)->table_size)

#define entry_count_(obj)   (header_ptr_(obj)->entry_count)


static inline size_t _table_byte_size(json_t *obj)
{
    return header_ptr_(obj)->table_size * sizeof(entry_t_);
}

static inline size_t _content_byte_size(json_t *obj)
{
    return header_ptr_(obj)->buf_size - 
        (sizeof(header_t_) + header_ptr_(obj)->table_size * sizeof(entry_t_));
}

/*******************************************************************************
 * Core Utility functions
 ******************************************************************************/

int32_t json_hash(char *str)
{
    char *ptr = str;
    size_t len = 0;
    int32_t result = *ptr << 7;
    for (; *ptr != '\0'; ptr++)
    {
        char cha = *ptr;
        result = (1000003 * result) ^ cha; //XOR
        len += 1;
    }
    result ^= len;
    return result;	
}

/*******************************************************************************
 * lower-level basic functions
 ******************************************************************************/

json_t json_init(void *buffer, size_t buf_size, size_t table_size)
{
    // Check if the buffer size is enough
    if (buf_size < (sizeof(header_t_) + table_size * sizeof(entry_t_)))
    {
        return (json_t){0};
    }
    // clear buffer
    memset(buffer, 0, buf_size);
    json_t new_obj = { 
        .buf = buffer
    };
    header_t_ *header = header_ptr_(&new_obj);
    *header = (header_t_){
        .buf_size = buf_size,
        .buf_idx = sizeof(header_t_) + table_size * sizeof(entry_t_),
        .table_size = table_size,
        .entry_count = 0
    };
    return new_obj;
}

int json_delete(json_t *obj, char *key)
{
    int idx = get_idx_(obj, key);
    if (idx < 0)
    {
        return JSON_NO_MATCHED_KEY;
    }
    // make a replicated object
    uint8_t buf_tmp[buf_size_(obj)];
    json_t tmp_obj = json_copy(buf_tmp, obj);
    
    // erase selected entry in the replica and then replace it
    memset((table_ptr_(&tmp_obj) + idx), 0, sizeof(entry_t_));

    // Clear original table and replace it again
    json_clear(obj);
    
    for (size_t i = 0; i < header_ptr_(&tmp_obj)->table_size; i++)
    {
        entry_t_ *entry = (table_ptr_(&tmp_obj) + i);
        if (NULL == entry->key)
        {
            continue;
        }
        json_insert(obj, entry->key, entry->value_ptr, entry->value_type);
    }
    
    return JSON_OK;
}

int json_clear(json_t *obj)
{
    // clear content
    memset(content_ptr_(obj), 0, _content_byte_size(obj));
    buf_idx_(obj) = sizeof(header_t_) + table_size_(obj) * sizeof(entry_t_);
    
    // clear table
    memset(table_ptr_(obj), 0, _table_byte_size(obj));
    entry_count_(obj) = 0;
    return JSON_OK;
}

/*******************************************************************************
 * Insertion functions
 ******************************************************************************/

int json_insert(json_t *obj, char *key, char *value, json_value_t type)
{
    switch (type)
    {
    case JSON_INT:
        return json_insert_int(obj, key, *(int *)value);
        break;
    case JSON_FLOAT:
        return json_insert_float(obj, key, *(float *)value);
        break;
    case JSON_STRING:
        return json_insert_str(obj, key, value);
    default:
        return JSON_ERROR;
    }
}

int json_insert_int(json_t *obj, char *key, int32_t value)
{    
    return insert_(obj, key, &value, sizeof(int), JSON_INT);
}

int json_insert_float(json_t *obj, char *key, float value)
{    
    return insert_(obj, key, &value, sizeof(float), JSON_FLOAT);
}

int json_insert_str(json_t *obj, char *key, char *value)
{
    // make temporary string. Length is multiples of 8.
    int size = (((strlen(value) + 1) >> 3) + 1) << 3;
    char str_tmp[size];
    memset(str_tmp, 0, size);
    strcpy(str_tmp, value);
    
    return insert_(obj, key, str_tmp, size, JSON_STRING);
}

/*******************************************************************************
 * Getter functions
 ******************************************************************************/

void *json_get(json_t *obj, char *key, json_value_t type)
{
    int idx = get_idx_(obj, key);
    if (idx >= 0)
    {
    	json_value_t target_type = table_ptr_(obj)[idx].value_type;
        return (target_type == type)? table_ptr_(obj)[idx].value_ptr : NULL;
    }
    return NULL;
}

int json_get_int(json_t *obj, char *key)
{
    void *result = json_get(obj, key, JSON_INT);
    return (NULL != result) ? *(int *)result : 0;
}

float json_get_float(json_t *obj, char *key)
{
    void *result = json_get(obj, key, JSON_FLOAT);
    return (NULL != result) ? *(float *)result : 0.0;
}

char *json_get_str(json_t *obj, char *key)
{
    return (char *)json_get(obj, key, JSON_STRING);
}

/*******************************************************************************
 * Setter functions
 ******************************************************************************/

int json_set(json_t *obj, char *key, void *value)
{
    int idx = get_idx_(obj, key);
    if (idx >= 0)
    {
        entry_t_ *entry = table_ptr_(obj) + idx;
        memcpy(entry->value_ptr, value, entry->value_size);
        return JSON_OK;
    }
    return JSON_ERROR;
}

int json_set_str(json_t *obj, char *key, char *value)
{
    // make temporary string. Length is multiples of 8.
    int size = (((strlen(value) + 1) >> 3) + 1) << 3;
    char str_tmp[size];
    memset(str_tmp, 0, size);
    strcpy(str_tmp, value);
    
    int idx = get_idx_(obj, key);
    if (idx >= 0)
    {
        entry_t_ *entry = table_ptr_(obj) + idx;
        if (size > entry->value_size)
        {
            return JSON_ENTRY_BUFFER_FULL;
        }
        memcpy(entry->value_ptr, value, entry->value_size);
        return JSON_OK;
    }
    return JSON_ERROR;
}

int json_set_int(json_t *obj, char *key, int value)
{
    return json_set(obj, key, &value);
}

int json_set_float(json_t *obj, char *key, float value)
{
    return json_set(obj, key, &value);
}

/*******************************************************************************
 * Buffer and memory management functions
 ******************************************************************************/

int json_replace_buffer(json_t *obj, void *new_buf, size_t size)
{
    void *old_buf = obj->buf;
    // Because in some system the offset is beyond signed int
    // TODO: check if long int is always 32 bit in 32 bit system and 64 bit in 64 bit system
    char is_plus = ((new_buf - old_buf) > 0)? 1: 0;
    size_t offset = (is_plus)?(new_buf - old_buf):(old_buf - new_buf);
    for (size_t i = 0; i < table_size_(obj); i++)
    {
        entry_t_ *entry = (table_ptr_(obj) + i);
        if (NULL == entry->key)
        {
            continue;
        }
        if (is_plus)
        {
            entry->key += offset;
            entry->value_ptr += offset;
        }
        else
        {
            entry->key -= offset;
            entry->value_ptr -= offset;
        }
    }
    // clear buffer first then copy
    memset(new_buf, 0, size);
    memcpy(new_buf, old_buf, buf_idx_(obj));
    
    // move pointers
    obj->buf = new_buf;
    
    // Confirm
    buf_size_(obj) = size;
    return JSON_OK;
}


int json_double_table(json_t *obj)
{
    // Check if the buffer is big enough
    if (buf_size_(obj) - buf_idx_(obj) < _table_byte_size(obj))
    {
        return JSON_BUFFER_FULL;
    }
    // create a temp JSON object
    uint8_t tmp_buf[buf_size_(obj)];
    json_t tmp_obj = json_init(tmp_buf, buf_size_(obj), table_size_(obj) * 2);
    
    for (size_t i = 0; i < table_size_(obj); i++)
    {
        entry_t_ *entry = (table_ptr_(obj) + i);
        if (NULL == entry->key)
        {
            continue;
        }
        json_insert(&tmp_obj, entry->key, entry->value_ptr, entry->value_type);
    }
    // Then replace buffer
    json_replace_buffer(&tmp_obj, obj->buf, buf_size_(obj));
    
    // Then replace table
    obj->buf = tmp_obj.buf;
    return JSON_OK;
}

json_t json_copy(void *dest_buf, json_t *obj)
{
    // Because in some system the offset is beyond signed int
    // TODO: check if long int is always 32 bit in 32 bit system and 64 bit in 64 bit system
    char is_plus = ((dest_buf - obj->buf) > 0)? 1: 0;
    size_t offset = (is_plus)?(dest_buf - obj->buf):(obj->buf - dest_buf);
    memcpy(dest_buf, obj->buf, buf_size_(obj));
    json_t new_obj = { 
        .buf = dest_buf
    };
    for (size_t i = 0; i < table_size_(obj); i++)
    {
        if (NULL == table_ptr_(obj)[i].key)
        {
            continue;
        }
        if (is_plus)
        {
            table_ptr_(&new_obj)[i].key += offset;
            table_ptr_(&new_obj)[i].value_ptr += offset;
        }
        else
        {
            table_ptr_(&new_obj)[i].key -= offset;
            table_ptr_(&new_obj)[i].value_ptr -= offset;
        }
    }
    return new_obj;
}

/*******************************************************************************
 * Other utility functions
 ******************************************************************************/

size_t json_table_size(json_t *obj)
{
    return table_size_(obj);
}

size_t json_count(json_t *obj)
{
    return entry_count_(obj);
}

size_t json_buffer_size(json_t *obj)
{
    return buf_size_(obj);
}

/*******************************************************************************
 * Private functions
 ******************************************************************************/

static int get_idx_(json_t *obj, char *key)
{
    int32_t hash = json_hash(key);
    size_t idx = hash & (table_size_(obj) - 1);
    entry_t_ entry;
    
    // variables for open addressing
    int perturb = hash;
    uint8_t checked[table_size_(obj)];
    size_t count = 0;
    
    // Search start
    entry = table_ptr_(obj)[idx];
    if (hash == entry.hash)
    {   // Good
        goto end_success;
    }
    
    // On collision
    memset(checked, 0, table_size_(obj));
    while (hash != entry.hash)
    {
        if (0 == checked[idx])
        {
            count++;
            if (count >= table_size_(obj))
            {   // When the table is full and visited all entries
                goto end_error;
            }
        }
        if (NULL == entry.key)
        {   // When we get empty slot
            goto end_error;
        }
        // open addressing
        idx = (5 * idx) + 1 + perturb;
        perturb >>= PERTURB_SHIFT;
        idx = idx & (table_size_(obj) - 1);
        // refresh
        entry = table_ptr_(obj)[idx];
    }
end_success:
    return idx;
end_error:
    return JSON_NO_MATCHED_KEY;
}


static int insert_(json_t *obj, char *key, void *value, size_t size, json_value_t type)
{
    if (entry_count_(obj) >= table_size_(obj))
    {
        return JSON_TABLE_FULL;
    }
    
    // buffer size check
    size_t key_size = strlen(key) + 1;
    size_t value_size = size + 1;
    size_t buf_required = key_size + value_size;
    if (buf_idx_(obj) + buf_required > buf_size_(obj))
    {
        return JSON_BUFFER_FULL;
    }
    
    // construct entry object first, with hash.
    entry_t_ new_entry = {0};
    new_entry.hash = json_hash(key);
    
    // put into the table
    int32_t new_idx = new_entry.hash & (table_size_(obj) - 1);
    
    int perturb = new_entry.hash;
    while (NULL != (table_ptr_(obj)[new_idx].key))
    {	// collision, open addressing
        if (new_entry.hash == table_ptr_(obj)[new_idx].hash)
        {	// collision, and the hash are the same (same key)
            return JSON_KEY_EXISTS;
        }
        new_idx = (new_idx << 2) + new_idx + 1 + perturb;
        perturb >>= PERTURB_SHIFT;
        new_idx = new_idx & (table_size_(obj) - 1);
    }
    
    // then put key into the buffer
    void *key_ptr = obj->buf + buf_idx_(obj);
    strcpy((char *)key_ptr, key);
    new_entry.key = key_ptr;
    buf_idx_(obj) += key_size;
    
    // put value into the buffer
    void *value_ptr = obj->buf + buf_idx_(obj);
    memcpy(value_ptr, value, size);
    new_entry.value_ptr = value_ptr;
    buf_idx_(obj) += value_size;
    
    // Put the new entry
    new_entry.value_type = type;
    new_entry.value_size = value_size;
    table_ptr_(obj)[new_idx] = new_entry;
    
    entry_count_(obj) += 1;
    return JSON_OK;
}

/**
 * Undefine macros
 */

// pointer macros
#undef  header_ptr_
#undef table_ptr_
#undef content_ptr_

// pointer size functions
#undef buf_size_

#undef buf_idx_

#undef table_size_

#undef entry_count_
