emJSON - JSON for embedded systems
==================================

_emJSON is a fast and memory efficient JSON library for embedded systems, written in C99_

There are two libraries in this project:
* `json.h`: Low-level JSON library that does not use dynamic memory allocation (e.g. malloc().)
* `emJSON.h` : Easy-to-use JSON library that is built open `json.h` functions and make use of dynamic memory allocation.

Features
--------

* Fast and efficient hash map algorithm.
* Minimized use of memory and memory fragmentation.
* No use of malloc() (specific to json.h)
* No library dependencies.
* Only need to include a single file (emJSON.h or json.h)

Examples
--------

### emJSON.h:
``` C
json_t test = emJSON_init();
emJSON_insert_str(&test, "string", "JSON Is Cool");
emJSON_insert_int(&test, "integer", 142);
emJSON_insert_float(&test, "float", 0.0456);

char *str = emJSON_string(&test);
printf("%s\n", str);
free(str);

/*
{"integer":142,"float":0.045600,"string":"JSON Is Cool"}
 */

printf("%s\n", emJSON_get_str(&test, "string"));
printf("%d\n", emJSON_get_int(&test, "integer"));
printf("%f\n", emJSON_get_float(&test, "float"));

/*
JSON Is Cool
142
0.045600
*/
emJSON_free(&test);

```

### json.h:
``` C
char buf[150];

json_t test = json_init(buf, 256, 4);  // 8 is table size
json_insert_str(&test, "string", "JSON Is Cool");
json_insert_int(&test, "integer", 142);
json_insert_float(&test, "float", 0.0456);

char str[60];
json_strcpy(str, &test);
printf("%s\n", str);

/*
{"float":0.045600,"string":"JSON Is Cool","integer":142}
 */

printf("%s\n", json_get_str(&test, "string"));
printf("%d\n", json_get_int(&test, "integer"));
printf("%f\n", json_get_float(&test, "float"));

/*
JSON Is Cool
142
0.045600
*/
```

Implementation
--------------

The algorithm is highly inspired by [Python's Dictionary implementation](http://svn.python.org/projects/python/trunk/Objects/dictobject.c) and [Python's string hash algorithm](https://svn.python.org/projects/python/trunk/Objects/stringobject.c).


TODOs
--------

* [ ] JSON encoding
* [x] JSON decoding
* [ ] Thread-safe
* [ ] Data alignment
* [x] Support String type
* [x] Support Integer type
* [x] Support Number type (floating point)
* [ ] Support object type
* [ ] Support Boolean literals (true, false)
* [ ] Support Null literal (null)
* [ ] Support array type

Limitations
-----------

### Common
* The format of JSON should be correct. Otherwise, the code will break.
* No comment in JSON allowed.


### json.h specific

Because it does not use dynamic memory allocations there are several limitation:

* buffer should be big enough that it does not have overflow or `json_replace_buffer()` to move to a bigger buffer.
* json entry size is not auto-resizable. Therefore use big enough json entry table, or `json_replace_table()` to move to a bigger table.
* The json entry size SHOULD be a power of 2. (e.g. 2, 4, 8, 16...)
* Buffer size of string value is a multiple of 8. If you change string value over than string buffer size, run `json_remove()` then `json_insert()`.