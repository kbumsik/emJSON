#include "json.h"
#include <string.h>

#define PERTURB_SHIFT 5


json_t json_init(void *buffer, size_t buf_size, json_entry_t *table, size_t table_size)
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
	// construct entry object first, with hash.
	json_entry_t new_entry = { 
		.key = key,
		.value_ptr = value
	};
	new_entry.hash = json_hash(key);
	
	// put into the table
	int32_t new_idx = new_entry.hash & (obj->entry_size - 1);
	
	int perturb = new_entry.hash;
	while (NULL != (obj->entry_table[new_idx].key))
	{	// collosion, open addressing
		if (new_entry.hash == obj->entry_table[new_idx].hash)
		{	// collosion, and the hashs are the same (same key)
			return JSON_KEY_EXISTS;
		}
		new_idx = (5 * new_idx) + 1 + perturb;
		perturb >>= PERTURB_SHIFT;
		new_idx = new_idx & (obj->entry_size - 1);
	}
	
	// buffer size check
	int buf_required = (strlen(key) + 1) + (strlen(value) + 1);
	if (obj->buf_idx + buf_required > obj->buf_size)
	{
		return JSON_BUFFER_FULL;
	}
	// Put the new entry
	obj->entry_table[new_idx] = new_entry;
	
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
	
	obj->entry_count += 1;
	return JSON_OK;
}

void *json_get(json_t *obj, char *key)
{
	// TODO: match the key too
	int32_t hash = json_hash(key);
	int idx = hash & (obj->entry_size - 1);
	json_entry_t entry = obj->entry_table[idx];
	
	int perturb = hash;
	int count = 0;
	while (hash != entry.hash)
	{
		count += 1;
		if (count > obj->entry_size)
		{
			return NULL;
		}
		// open addressing
		idx = (5 * idx) + 1 + perturb;
		perturb >>= PERTURB_SHIFT;
		idx = idx & (obj->entry_size - 1);
		// refresh
		entry = obj->entry_table[idx];
	}
	void *result = entry.value_ptr;
	return result;
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

