#include "json.h"
#include <string.h>
#include <stdio.h>
#include "json_internal.h"

#define PERTURB_SHIFT 5

// Private functions

struct result_
{
	int status;
	size_t idx;
};

static int get_idx_(json_t *obj, char *key);
static struct result_ insert_(json_t *obj, char *key, void *value, size_t size, json_type_t type);
static void table_move_ptr_ (void *dest, void *source, json_t *obj);

/*******************************************************************************
 * Core Hash function
 ******************************************************************************/

/*
 * Hash settings
 */

// Hash functions
#define EMJSON_HASH(hash, cha) EMJSON_JAVA_HASH(hash, cha)
    // 1000003 from python dictionary implementation,
    #define EMJSON_PYTHON_HASH(hash, cha)    ((1000003 * hash) ^ cha)
    // 31 from Java Hashmap
    #define EMJSON_JAVA_HASH(hash, cha)        ((hash << 5) - hash + cha)
    // 101 from Microsoft Research.
    #define EMSJOSN_MS_HASH(hash, cha)        ((101 * hash) + cha)

// Hash starting value
#define EMJSON_HASH_START(cha) EMJSON_JAVA_HASH_START(cha)
    // 1000003 from python dictionary implementation,
    #define EMJSON_PYTHON_HASH_START(cha) (cha << 7)
    // 31 from Java Hashmap
    #define EMJSON_JAVA_HASH_START(cha) (0)
    // 101 from Microsoft Research.
    #define EMSJOSN_MS_HASH_START(cha) (0)

int32_t json_hash(char *str)
{
    char *ptr = str;
    size_t len = 0;
    int32_t result = EMJSON_HASH_START(*ptr);
    for (; *ptr != '\0'; ptr++)
    {
        char cha = *ptr;
        result = EMJSON_HASH(result, cha);
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
    if (buf_size < (sizeof(struct header_) + table_size * sizeof(struct entry_)))
    {
        return (json_t){0};
    }
    // clear buffer
    memset(buffer, 0, buf_size);
    json_t new_obj = { 
        .buf = buffer
    };
    struct header_ *header = header_ptr_(&new_obj);
    *header = (struct header_){
    	.parent = NULL,
        .buf_size = buf_size,
        .buf_idx = sizeof(struct header_) + table_size * sizeof(struct entry_),
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
    memset((table_ptr_(&tmp_obj) + idx), 0, sizeof(struct entry_));

    // Clear original table and replace it again
    json_clear(obj);
    
    for (size_t i = 0; i < header_ptr_(&tmp_obj)->table_size; i++)
    {
        struct entry_ *entry = (table_ptr_(&tmp_obj) + i);
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
    memset(content_ptr_(obj), 0, content_byte_size_(obj));
    buf_idx_(obj) = sizeof(struct header_) + table_size_(obj) * sizeof(struct entry_);
    
    // clear table
    memset(table_ptr_(obj), 0, table_byte_size_(obj));
    entry_count_(obj) = 0;
    return JSON_OK;
}

/*******************************************************************************
 * Insertion functions
 ******************************************************************************/

int json_insert(json_t *obj, char *key, void *value, json_type_t type)
{
    switch (type)
    {
    case JSON_INT:
        return json_insert_int(obj, key, *(int *)value);
    case JSON_FLOAT:
        return json_insert_float(obj, key, *(float *)value);
    case JSON_STRING:
        return json_insert_str(obj, key, value);
    case JSON_OBJECT:
        return json_insert_obj(obj, key, value);
    default:
        return JSON_ERROR;
    }
}

int json_insert_int(json_t *obj, char *key, int32_t value)
{    
    return insert_(obj, key, &value, sizeof(int), JSON_INT).status;
}

int json_insert_float(json_t *obj, char *key, float value)
{    
    return insert_(obj, key, &value, sizeof(float), JSON_FLOAT).status;
}

int json_insert_str(json_t *obj, char *key, char *value)
{
    // make temporary string. Length is multiples of 8.
    int size = (((strlen(value) + 1) >> 3) + 1) << 3;
    char str_tmp[size];
    memset(str_tmp, 0, size);
    strcpy(str_tmp, value);
    
    return insert_(obj, key, str_tmp, size, JSON_STRING).status;
}


int json_insert_obj(json_t *obj, char *key, json_t *input)
{
	// insert
	struct result_ ret = insert_(obj, key, input->buf, buf_size_(input), JSON_OBJECT);
	if (ret.status != JSON_OK)
	{
		return ret.status;
	}
	// move pointers
	table_move_ptr_ (table_ptr_(obj)[ret.idx].value_ptr, input->buf, input);

	// change input object and set parent
	// FIXME: make the following line shorter
	((struct header_ *)(table_ptr_(obj)[ret.idx].value_ptr))->parent_entry_idx = ret.idx;
	input->buf = table_ptr_(obj)[ret.idx].value_ptr;
	parent_ptr_(input) = obj->buf;
	return ret.status;
}


int json_insert_empty_obj(json_t *obj, char *key, size_t size)
{
	// insert
	struct result_ ret = insert_(obj, key, NULL, size, JSON_NULL);
	if (ret.status != JSON_OK)
	{
		return ret.status;
	}
	// get object and init
	json_t tmp = json_init(table_ptr_(obj)[ret.idx].value_ptr, size, 4);
	parent_ptr_(&tmp) = obj->buf;
	table_ptr_(obj)[ret.idx].value_type = JSON_OBJECT;
	((struct header_ *)(table_ptr_(obj)[ret.idx].value_ptr))->parent_entry_idx = ret.idx;
	// TODO: what if the buffer is not enough?
	return ret.status;
}

/*******************************************************************************
 * Getter functions
 ******************************************************************************/

void *json_get(json_t *obj, char *key, json_type_t type)
{
    int idx = get_idx_(obj, key);
    if (idx >= 0)
    {
    	json_type_t target_type = table_ptr_(obj)[idx].value_type;
        return (target_type == type && target_type != JSON_NULL)? table_ptr_(obj)[idx].value_ptr : NULL;
    }
    return NULL;
}

int json_get_int(json_t *obj, char *key)
{
    void *result = json_get(obj, key, JSON_INT);
    return (NULL != result) ? (*(int *)result) : 0;
}

float json_get_float(json_t *obj, char *key)
{
    void *result = json_get(obj, key, JSON_FLOAT);
#ifdef __CC_ARM
    // in uVision(keil) and mbed compiler cause hardfault when
    // just assigned like:
    // value = *(float *)result;
    // the VLDR instruction causes hard fault. I don't know why.
    // So I use memcpy() instead.
    float value;
    memcpy(&value, result, sizeof(4));
    return value;
#else // like __GNUC__
    return (NULL != result) ? (*(float *)result) : 0;
#endif
}

char *json_get_str(json_t *obj, char *key)
{
    return (char *)json_get(obj, key, JSON_STRING);
}

json_t json_get_obj(json_t *obj, char *key)
{
	json_t ret = {
			.buf = json_get(obj, key, JSON_OBJECT)
	};
    return ret;
}

/*******************************************************************************
 * Setter functions
 ******************************************************************************/

int json_set(json_t *obj, char *key, void *value)
{
    int idx = get_idx_(obj, key);
    if (idx >= 0)
    {
        struct entry_ *entry = table_ptr_(obj) + idx;
        memcpy(entry->value_ptr, value, entry->value_size);
        return JSON_OK;
    }
    return JSON_ERROR;
}

int json_set_str(json_t *obj, char *key, char *value)
{
    // make temporary string. Length is multiples of 8.
    size_t size = (((strlen(value) + 1) >> 3) + 1) << 3;
    char str_tmp[size];
    memset(str_tmp, 0, size);
    strcpy(str_tmp, value);
    
    int idx = get_idx_(obj, key);
    if (idx >= 0)
    {
        struct entry_ *entry = table_ptr_(obj) + idx;
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

    table_move_ptr_ (new_buf, old_buf, obj);

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
    if (buf_size_(obj) - buf_idx_(obj) < table_byte_size_(obj))
    {
        return JSON_BUFFER_FULL;
    }
    // create a temp JSON object
    uint8_t tmp_buf[buf_size_(obj)];
    json_t tmp_obj = json_init(tmp_buf, buf_size_(obj), table_size_(obj) * 2);
    
    for (size_t i = 0; i < table_size_(obj); i++)
    {
        struct entry_ *entry = (table_ptr_(obj) + i);
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
    memcpy(dest_buf, obj->buf, buf_size_(obj));
    json_t new_obj = { 
        .buf = dest_buf
    };

    table_move_ptr_ (dest_buf, obj->buf, &new_obj);
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
 * Debug functions
 ******************************************************************************/

#ifndef DEBUG
	// empty
#else
void json_debug_print_obj(json_t *obj)
{
	if (NULL == obj->buf)
	{
	    JSON_DEBUG_PRINTF("Invalid object!! buffer is NULL!\n");
	    return;
	}
    JSON_DEBUG_PRINTF("==========================\n");
    JSON_DEBUG_PRINTF("\t emJSON object block\n");
    JSON_DEBUG_PRINTF("==========================\n");
    JSON_DEBUG_PRINTF("Parent: %p\n", parent_ptr_(obj));
    JSON_DEBUG_PRINTF("Table index in Parent: %u\n", (unsigned int)idx_in_parent_(obj));
    JSON_DEBUG_PRINTF("Buffer size: 0x%x\n", (unsigned int)buf_size_(obj));
    JSON_DEBUG_PRINTF("Current buffer index: 0x%x\n", (unsigned int)buf_idx_(obj));
    JSON_DEBUG_PRINTF("Size of table : %lu\n", table_size_(obj));
    JSON_DEBUG_PRINTF("Entry count : %lu\n", entry_count_(obj));
    // Print pointers
    JSON_DEBUG_PRINTF("Header Pointer : %p\n", header_ptr_(obj));
    JSON_DEBUG_PRINTF("Size of header in bytes: 0x%x\n", (unsigned int)sizeof(struct header_));
    JSON_DEBUG_PRINTF("Table Pointer : %p\n", table_ptr_(obj));
    JSON_DEBUG_PRINTF("Size of each entry in the table: 0x%x\n", (unsigned int)sizeof(struct entry_));
    JSON_DEBUG_PRINTF("Size of table in bytes: 0x%x\n", (unsigned int)table_byte_size_(obj));
    JSON_DEBUG_PRINTF("Content Pointer : %p\n", content_ptr_(obj));
    JSON_DEBUG_PRINTF("Size of Content in bytes: 0x%x\n", (unsigned int)content_byte_size_(obj));

    for (int i = 0; i < (int)header_ptr_(obj)->table_size; i++)
    {
        struct entry_ *entry = (table_ptr_(obj) + i);
        JSON_DEBUG_PRINTF("--------------------------\n");
        JSON_DEBUG_PRINTF("\t Entry %d\n", i);
        JSON_DEBUG_PRINTF("--------------------------\n");
        if (NULL == entry->key)
        {
            JSON_DEBUG_PRINTF("ENTRY IS EMPTY\n");
            continue;
        }
        JSON_DEBUG_PRINTF("Key : %s\n", entry->key);
        JSON_DEBUG_PRINTF("Key pointer: %p\n", (void *)entry->key);
        JSON_DEBUG_PRINTF("Key length: 0x%x\n", (unsigned int)strlen(entry->key) + 1);
        JSON_DEBUG_PRINTF("Hash : %u\n", entry->hash);
        JSON_DEBUG_PRINTF("Value pointer : %p\n", entry->value_ptr);
        JSON_DEBUG_PRINTF("Value size in bytes: 0x%x\n", (unsigned int)entry->value_size);
        // print type
        switch(entry->value_type)
        {
        case JSON_INT:
            JSON_DEBUG_PRINTF("Entry type : Integer\n");
            JSON_DEBUG_PRINTF("Value : %d\n", *(int *)entry->value_ptr);
            break;
        case JSON_FLOAT:
            JSON_DEBUG_PRINTF("Entry type : Floating Point\n");
            JSON_DEBUG_PRINTF("Value : %f\n", *(float *)entry->value_ptr);
            break;
        case JSON_STRING:
            JSON_DEBUG_PRINTF("Entry type : String\n");
            JSON_DEBUG_PRINTF("Charters count : 0x%x\n", (unsigned int)strlen((char *)entry->value_ptr));
            JSON_DEBUG_PRINTF("Value : %s\n", (char *)entry->value_ptr);
            break;
        case JSON_OBJECT:
            JSON_DEBUG_PRINTF("Entry type : Object\n");
            JSON_DEBUG_PRINTF("======= Child Object Printing =======\n");
            {
            	json_t tmp = {.buf = entry->value_ptr};
            	json_debug_print_obj(&tmp);
            }
            JSON_DEBUG_PRINTF("======= Child Object Printing End =======\n");
            break;
        default:
            JSON_DEBUG_PRINTF("UNKOWN TYPE!!!\n");
        }
        // print dump
        char dump_str[17];
        memset(dump_str, 0, 17);
        // key dump
        JSON_DEBUG_PRINTF("Key dump\n");
        uint8_t *dump = (uint8_t *)entry->key;
        for (size_t j = 0; j < strlen(entry->key)+1; j++)
        {
            sprintf(&dump_str[(2*j)%16], "%02x", (uint8_t)dump[j]);
            if( ((j%8 == 7)) || (j == (strlen(entry->key))) )
            {
                JSON_DEBUG_PRINTF("%s\n", dump_str);
                memset(dump_str, 0, 17);
            }
        }
        // value dump
        JSON_DEBUG_PRINTF("Value dump\n");
        dump = (uint8_t *)entry->value_ptr;
        for (size_t j = 0; j < entry->value_size; j++)
        {
            sprintf(&dump_str[(2*j)%16], "%02x", (uint8_t)dump[j]);
            if( ((j%8 == 7)) || (j == (entry->value_size -1)) )
            {
                JSON_DEBUG_PRINTF("%s\n", dump_str);
                memset(dump_str, 0, 17);
            }
        }
        memset(dump_str, 0, 17);
    }
    JSON_DEBUG_PRINTF("==========================\n");
    JSON_DEBUG_PRINTF("\t End printing\n");
    JSON_DEBUG_PRINTF("==========================\n");
    return;
#endif
}

/*******************************************************************************
 * Private functions
 ******************************************************************************/

static int get_idx_(json_t *obj, char *key)
{
    int32_t hash = json_hash(key);
    size_t idx = hash & (table_size_(obj) - 1);
    struct entry_ entry;
    
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
        	checked[idx] += 1;
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


static struct result_ insert_(json_t *obj, char *key, void *value, size_t size, json_type_t type)
{
	struct result_ ret = {
			.status = JSON_ERROR,
			.idx = 0
	};
    if (entry_count_(obj) >= table_size_(obj))
    {
    	ret.status = JSON_TABLE_FULL;
        return ret;
    }
    
    // buffer size check
    size_t key_size = strlen(key) + 1;
    size_t value_size = size;
    size_t buf_required = key_size + value_size;
    if (buf_idx_(obj) + buf_required > buf_size_(obj))
    {
    	ret.status = JSON_BUFFER_FULL;
        return ret;
    }
    
    // construct entry object first, with hash.
    struct entry_ new_entry = {0};
    new_entry.hash = json_hash(key);
    
    // put into the table
    int32_t new_idx = new_entry.hash & (table_size_(obj) - 1);
    
    int perturb = new_entry.hash;
    while (NULL != (table_ptr_(obj)[new_idx].key))
    {    // collision, open addressing
        if (new_entry.hash == table_ptr_(obj)[new_idx].hash)
        {    // collision, and the hash are the same (same key)
        	ret.status = JSON_KEY_EXISTS;
            return ret;
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
    if (NULL != value)
    {
        memcpy(value_ptr, value, size);
    }
    new_entry.value_ptr = value_ptr;
    buf_idx_(obj) += value_size;
    
    // Put the new entry
    new_entry.value_type = (NULL != value)?type:JSON_NULL;
    new_entry.value_size = value_size;
    table_ptr_(obj)[new_idx] = new_entry;
    
    entry_count_(obj) += 1;

    ret.status = JSON_OK;
    ret.idx = new_idx;
    return ret;
}

static void table_move_ptr_ (void *dest, void *source, json_t *obj)
{
    // Because in some system the offset is beyond signed int
    char is_plus = ((dest - source) > 0)? 1: 0;
    size_t offset = (is_plus)?(dest - source):(source - dest);
    for (size_t i = 0; i < table_size_(obj); i++)
    {
        struct entry_ *entry = (table_ptr_(obj) + i);
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
}
