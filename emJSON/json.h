#ifndef __JSON_H__
#define __JSON_H__

#include <stdint.h>
#include <stdlib.h>

#define JSON_OK             		0
#define JSON_ERROR          		-1   // Unknown error
#define JSON_NO_MATCHED_KEY			-2
#define JSON_KEY_EXISTS	    		-3
#define JSON_TABLE_FULL	    		-4
#define JSON_BUFFER_FULL    		-5
#define JSON_ENTRY_BUFFER_FULL		-6
#define JSON_TYPE_MISMATCH			-7

typedef uint8_t json_type_t;
	#define JSON_INT		1
	#define JSON_FLOAT		2
	#define JSON_STRING		3
	#define JSON_OBJECT		4
	//#define JSON_ARRAY	5
	#define JSON_NULL		6
	//#define JSON_BOOL		7
	#define JSON_UNKNOWN	8

typedef struct{
    void *buf;
} json_t;

#ifdef __cplusplus
extern "C"{
#endif

// core utility functions
int32_t json_hash(char *str);

// lower-level basic functions
json_t json_init(void *buffer, size_t buf_size, size_t table_size);
int json_delete(json_t *obj, char *key);
int json_clear(json_t *obj);

// Insertion functions
int json_insert(json_t *obj, char *key, void *value, json_type_t type);
int json_insert_str(json_t *obj, char *key, char *value);
int json_insert_int(json_t *obj, char *key, int32_t value);
int json_insert_float(json_t *obj, char *key, float value);
int json_insert_obj(json_t *obj, char *key, json_t *input);
int json_insert_empty_obj(json_t *obj, char *key, size_t size);	// make it internal?

// Getter functions
void  *json_get(json_t *obj, char *key, json_type_t type);
char  *json_get_str(json_t *obj, char *key);
int	   json_get_int(json_t *obj, char *key);
float  json_get_float(json_t *obj, char *key);
json_t json_get_obj(json_t *obj, char *key);

// Setter functions
int json_set(json_t *obj, char *key, void *value);
int json_set_str(json_t *obj, char *key, char *value);
int json_set_int(json_t *obj, char *key, int value);
int json_set_float(json_t *obj, char *key, float value);

// String-related functions
int json_parse(json_t *obj, char *input);
int json_strcpy(char *dest, json_t *obj);
int json_strlen(json_t *obj);

// Buffer and memory management functions
int json_replace_buffer(json_t *obj, void *new_buf, size_t size);
int json_double_table(json_t *obj);
json_t json_copy(void *dest_buf, json_t *obj);

// Other utility functions
size_t json_table_size(json_t *obj);
size_t json_count(json_t *obj);
size_t json_buffer_size(json_t *obj);


// Debugging Support
#ifdef DEBUG
    #include <stdio.h>
	void json_debug_print_obj(json_t *obj);
	#define JSON_DEBUG_PRINT_OBJ(obj)       json_debug_print_obj(obj)

	#if defined(ARDUINO_PLATFORM) || defined(ARDUINO)
		#define JSON_DEBUG_PRINTF(msg, ...)     printf("EMJSON-DEBUG: " msg, ##__VA_ARGS__)
	#else
		#define JSON_DEBUG_PRINTF(msg, ...)     printf("EMJSON-DEBUG: " msg, ##__VA_ARGS__)
	#endif
#else
	#define JSON_DEBUG_PRINT_OBJ(obj)
    #define JSON_DEBUG_PRINTF(msg, ...)
#endif


#ifdef __cplusplus
}
#endif

#endif  // __JSON_H__

