#ifndef __JSON_H__
#define __JSON_H__

#include <stdint.h>
#include <stdlib.h>

#define JSON_OK			0
#define JSON_KEY_EXISTS	1
#define JSON_TABLE_FULL	2
#define JSON_BUFFER_FULL 3

typedef enum
{
	//integer,
	//number,
	string
	//obejct,
	//arrary,
	//boolean,
	//null_value
}json_value_e;

typedef struct
{
	char *key;
	size_t size;
	int32_t hash;
	void *value_ptr;
	//json_value_e value_type;
}json_entry_t;

typedef struct
{
	int		buf_size;
	void	*buf;
	size_t	buf_idx;
	json_entry_t *entry_table;
	size_t		entry_size;
	int		entry_count;
}json_t;

// lower-level functions
json_t json_init(void *buffer, size_t buf_size, json_entry_t *table, size_t table_size);
int json_insert(json_t *obj, char *key, void *value);
void *json_get(json_t *obj, char *key);
int json_set(json_t *obj, char *key, void *value);
int json_delete(json_t *obj, char *key, void *value);
int json_replace_buffer(void);
int json_strcpy(char *dest, json_t *obj);

// utility functions
int32_t json_hash(char *str);
int json_strlen(json_t *obj);

#endif	// __JSON_H__
