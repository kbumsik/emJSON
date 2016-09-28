#include "emJSON.h"

json_t emJSON_init()
{
	void *buffer = malloc(64);
	json_entry_t *table = (json_entry_t *)calloc(8, sizeof(json_entry_t));
	json_t result = json_init(buffer, 64, table, 8);
	return result;
}

int emJSON_insert(json_t *obj, char *key, void *value)
{
	return json_insert(obj, key, value); // TODO: Implement
}

void *emJSON_get(json_t *obj, char *key)
{
	return json_get(obj, key);
}

char *emJSON_string(json_t *obj)
{
	int len = json_strlen(obj) + 1;
	char *str = malloc(len * sizeof(char));
	json_strcpy(str, obj);
	return str;
}

int emJSON_free(json_t *obj)
{
	// free table
	free(obj->entry_table);
	// free buffer
	free(obj->buf);
	return 0;
}
