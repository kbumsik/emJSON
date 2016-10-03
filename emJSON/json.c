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
}_entry_t;

typedef struct
{
    size_t buf_size;
    size_t buf_idx;
    size_t table_size;
    size_t entry_count;
}_header_t;

// Private functions
static int _get_idx(json_t *obj, char *key);
static int _insert(json_t *obj, char *key, void *value, size_t size, json_value_t type);

// pointer macros
#define _header_ptr(obj)  ((_header_t *)((obj)->buf))
#define _table_ptr(obj)  ((_entry_t *)((obj)->buf + sizeof(_header_t)))
#define _content_ptr(obj) ((obj)->buf + sizeof(_header_t) + _table_byte_size(obj))

// pointer size functions
#define _buf_size(obj)  (_header_ptr(obj)->buf_size)

#define _buf_idx(obj)   (_header_ptr(obj)->buf_idx)

#define _table_size(obj)    (_header_ptr(obj)->table_size)

#define _entry_count(obj)   (_header_ptr(obj)->entry_count)


static inline size_t _table_byte_size(json_t *obj)
{
    return _header_ptr(obj)->table_size * sizeof(_entry_t);
}

static inline size_t _content_byte_size(json_t *obj)
{
    return _header_ptr(obj)->buf_size - 
        (sizeof(_header_t) + _header_ptr(obj)->table_size * sizeof(_entry_t));
}


/*******************************************************************************
 * Core Utility functions
 ******************************************************************************/

int32_t json_hash(char *str)
{
    char *ptr = str;
    int len = 0;
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
    if (buf_size < (sizeof(_header_t) + table_size * sizeof(_entry_t)))
    {
        return (json_t){0};
    }
    // clear buffer
    memset(buffer, 0, buf_size);
    json_t new_obj = { 
        .buf = buffer
    };
    _header_t *header = _header_ptr(&new_obj);
    *header = (_header_t){
        .buf_size = buf_size,
        .buf_idx = sizeof(_header_t) + table_size * sizeof(_entry_t),
        .table_size = table_size,
        .entry_count = 0
    };
    return new_obj;
}

int json_delete(json_t *obj, char *key)
{
    int idx = _get_idx(obj, key);
    if (idx < 0)
    {
        return JSON_NO_MATCHED_KEY;
    }
    // make a replicated object
    uint8_t buf_tmp[_buf_size(obj)];
    json_t tmp_obj = json_copy(buf_tmp, obj);
    
    // erase selected entry in the replica and then replace it
    memset((_table_ptr(&tmp_obj) + idx), 0, sizeof(_entry_t));

    // Clear original table and replace it again
    json_clear(obj);
    
    for (size_t i = 0; i < _header_ptr(&tmp_obj)->table_size; i++)
    {
        _entry_t *entry = (_table_ptr(&tmp_obj) + i);
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
    memset(_content_ptr(obj), 0, _content_byte_size(obj));
    _buf_idx(obj) = sizeof(_header_t) + _table_size(obj) * sizeof(_entry_t);
    
    // clear table
    memset(_table_ptr(obj), 0, _table_byte_size(obj));
    _entry_count(obj) = 0;
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
    return _insert(obj, key, &value, 4, JSON_INT);
}

int json_insert_float(json_t *obj, char *key, float value)
{    
    return _insert(obj, key, &value, 4, JSON_FLOAT);
}

int json_insert_str(json_t *obj, char *key, char *value)
{
    // make temporary string. Length is multiples of 8.
    int size = (((strlen(value) + 1) >> 3) + 1) << 3;
    char str_tmp[size];
    memset(str_tmp, 0, size);
    strcpy(str_tmp, value);
    
    return _insert(obj, key, str_tmp, size, JSON_STRING);
}

/*******************************************************************************
 * Getter functions
 ******************************************************************************/

void *json_get(json_t *obj, char *key, json_value_t type)
{
    int idx = _get_idx(obj, key);
    if (idx >= 0)
    {
    	json_value_t target_type = _table_ptr(obj)[idx].value_type;
        return (target_type == type)? _table_ptr(obj)[idx].value_ptr : NULL;
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
    int idx = _get_idx(obj, key);
    if (idx >= 0)
    {
        _entry_t *entry = _table_ptr(obj) + idx;
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
    
    int idx = _get_idx(obj, key);
    if (idx >= 0)
    {
        _entry_t *entry = _table_ptr(obj) + idx;
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
 * String-related functions
 ******************************************************************************/

int json_strcpy(char *dest, json_t *obj)
{
    char str_buf[30];   // FIXME: Take more string length
    int idx = 0;
    memset(dest + idx, '{', 1);
    idx += 1;
    // start
    for (size_t i = 0; i < _table_size(obj); i++)
    {
        _entry_t entry = _table_ptr(obj)[i];
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
        switch (entry.value_type)
        {
        case JSON_INT:
            sprintf(str_buf, "%d", *(int *)entry.value_ptr);
            break;
        case JSON_FLOAT:
            sprintf(str_buf, "%f", *(float *)entry.value_ptr);
            break;
        case JSON_STRING:
            memset(dest + idx, '\"', 1);
            idx += 1;
            sprintf(str_buf, "%s", (char *)entry.value_ptr);
            break;
        default:
            return JSON_ERROR;
        }
        str_len = strlen(str_buf);
        strcpy(dest + idx, str_buf);
        strcpy(dest + idx + str_len, (JSON_STRING == entry.value_type) ? "\"," : ",");
        idx += str_len + ((JSON_STRING == entry.value_type)? 2 : 1);
    }
    // it's the end
    // -1 is to remove the last ','.
    strcpy(dest + idx -1, "}");
    return JSON_OK;
}

int json_strlen(json_t *obj)
{
    char str_buf[30];   // FIXME: Take more string length
    int idx = 0;
    idx += 1;	// '{'
    // start
    for (size_t i = 0; i < _table_size(obj); i++)
    {
        _entry_t entry = _table_ptr(obj)[i];
        if (NULL == entry.key)
        {	// empty entry
            continue;
        }
        // "<key>":
        idx += 1; // '\"'
        int str_len = strlen(entry.key);
        idx += str_len + 2;	// <key>":
        // "<value>",
        switch (entry.value_type)
        {
        case JSON_INT:
            sprintf(str_buf, "%d", *(int *)entry.value_ptr);
            break;
        case JSON_FLOAT:
            sprintf(str_buf, "%f", *(float *)entry.value_ptr);
            break;
        case JSON_STRING:
            idx += 1;	// '\"'
            sprintf(str_buf, "%s", (char *)entry.value_ptr);
            break;
        default:
            return JSON_ERROR;
        }
        str_len = strlen(str_buf);
        idx += str_len + ((JSON_STRING == entry.value_type) ? 2 : 1); // <value>",
    }
    // it's the end, '}'
    // consider removing the last ','.
    return idx;
}

/*******************************************************************************
 * Buffer and memory management functions
 ******************************************************************************/

int json_replace_buffer(json_t *obj, void *new_buf, size_t size)
{
    void *old_buf = obj->buf;
    // Because in some system the offset is beyond signed int
    // TODO: check if long int is always 32 bit in 32 bit system and 64 bit in 64 bit system
    int is_plus = ((new_buf - old_buf) > 0)? 1: 0;
    unsigned long int offset = (is_plus)?(new_buf - old_buf):(old_buf - new_buf);
    for (size_t i = 0; i < _table_size(obj); i++)
    {
        _entry_t *entry = (_table_ptr(obj) + i);
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
    memcpy(new_buf, old_buf, _buf_idx(obj));
    
    // move pointers
    obj->buf = new_buf;
    
    // Confirm
    _buf_size(obj) = size;
    return JSON_OK;
}


int json_double_table(json_t *obj)
{
    // Check if the buffer is big enough
    if (_buf_size(obj) - _buf_idx(obj) < _table_byte_size(obj))
    {
        return JSON_BUFFER_FULL;
    }
    // create a temp JSON object
    uint8_t tmp_buf[_buf_size(obj)];
    json_t tmp_obj = json_init(tmp_buf, _buf_size(obj), _table_size(obj) * 2);
    
    for (size_t i = 0; i < _table_size(obj); i++)
    {
        _entry_t *entry = (_table_ptr(obj) + i);
        if (NULL == entry->key)
        {
            continue;
        }
        json_insert(&tmp_obj, entry->key, entry->value_ptr, entry->value_type);
    }
    // Then replace buffer
    json_replace_buffer(&tmp_obj, obj->buf, _buf_size(obj));
    
    // Then replace table
    obj->buf = tmp_obj.buf;
    return JSON_OK;
}

json_t json_copy(void *dest_buf, json_t *obj)
{
    // Because in some system the offset is beyond signed int
    // TODO: check if long int is always 32 bit in 32 bit system and 64 bit in 64 bit system
    int is_plus = ((dest_buf - obj->buf) > 0)? 1: 0;
    unsigned long int offset = (is_plus)?(dest_buf - obj->buf):(obj->buf - dest_buf);
    memcpy(dest_buf, obj->buf, _buf_size(obj));
    json_t new_obj = { 
        .buf = dest_buf
    };
    for (size_t i = 0; i < _table_size(obj); i++)
    {
        if (NULL == _table_ptr(obj)[i].key)
        {
            continue;
        }
        if (is_plus)
        {
            _table_ptr(&new_obj)[i].key += offset;
            _table_ptr(&new_obj)[i].value_ptr += offset;
        }
        else
        {
            _table_ptr(&new_obj)[i].key -= offset;
            _table_ptr(&new_obj)[i].value_ptr -= offset;
        }
    }
    return new_obj;
}

/*******************************************************************************
 * Other utility functions
 ******************************************************************************/

int json_table_size(json_t *obj)
{
    return _table_size(obj);
}

int json_count(json_t *obj)
{
    return _entry_count(obj);
}

int json_buffer_size(json_t *obj)
{
    return _buf_size(obj);
}

/*******************************************************************************
 * Private functions
 ******************************************************************************/

static int _get_idx(json_t *obj, char *key)
{
    int32_t hash = json_hash(key);
    int idx = hash & (_table_size(obj) - 1);
    _entry_t entry;
    
    // variables for open addressing
    int perturb = hash;
    uint8_t checked[_table_size(obj)];
    size_t count = 0;
    
    // Search start
    entry = _table_ptr(obj)[idx];
    if (hash == entry.hash)
    {   // Good
        goto end_success;
    }
    
    // On collision
    memset(checked, 0, _table_size(obj));
    while (hash != entry.hash)
    {
        if (0 == checked[idx])
        {
            count++;
            if (count >= _table_size(obj))
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
        idx = idx & (_table_size(obj) - 1);
        // refresh
        entry = _table_ptr(obj)[idx];
    }
end_success:
    return idx;
end_error:
    return JSON_NO_MATCHED_KEY;
}


static int _insert(json_t *obj, char *key, void *value, size_t size, json_value_t type)
{
    if (_entry_count(obj) >= _table_size(obj))
    {
        return JSON_TABLE_FULL;
    }
    
    // buffer size check
    int key_size = strlen(key) + 1;
    int value_size = size + 1;
    size_t buf_required = key_size + value_size;
    if (_buf_idx(obj) + buf_required > _buf_size(obj))
    {
        return JSON_BUFFER_FULL;
    }
    
    // construct entry object first, with hash.
    _entry_t new_entry = {0};
    new_entry.hash = json_hash(key);
    
    // put into the table
    int32_t new_idx = new_entry.hash & (_table_size(obj) - 1);
    
    int perturb = new_entry.hash;
    while (NULL != (_table_ptr(obj)[new_idx].key))
    {	// collision, open addressing
        if (new_entry.hash == _table_ptr(obj)[new_idx].hash)
        {	// collision, and the hash are the same (same key)
            return JSON_KEY_EXISTS;
        }
        new_idx = (new_idx << 2) + new_idx + 1 + perturb;
        perturb >>= PERTURB_SHIFT;
        new_idx = new_idx & (_table_size(obj) - 1);
    }
    
    // then put key into the buffer
    void *key_ptr = obj->buf + _buf_idx(obj);
    strcpy((char *)key_ptr, key);
    new_entry.key = key_ptr;
    _buf_idx(obj) += key_size;
    
    // put value into the buffer
    void *value_ptr = obj->buf + _buf_idx(obj);
    memcpy(value_ptr, value, size);
    new_entry.value_ptr = value_ptr;
    _buf_idx(obj) += value_size;
    
    // Put the new entry
    new_entry.value_type = type;
    new_entry.value_size = value_size;
    _table_ptr(obj)[new_idx] = new_entry;
    
    _entry_count(obj) += 1;
    return JSON_OK;
}

/**
 * Undefine macros
 */

// pointer macros
#undef  _header_ptr
#undef _table_ptr
#undef _content_ptr

// pointer size functions
#undef _buf_size

#undef _buf_idx

#undef _table_size

#undef _entry_count
