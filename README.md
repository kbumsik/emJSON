emJSON - JSON for embedded systems
==================================

_emJSON is a fast and memory efficient JSON library for embedded systems, written in C99_

There are two libraries in this project:
* `json.h`: Low-level JSON library that does not use dynamic memory allocation (e.g. malloc().)
* `emJSON.h` : Easy-to-use JSON library that is built open `json.h` functions and make use of dynamic memory allocation.

Features
--------

* Fast and efficient hash map algorithm.
* Minimized use of memory.
* No use of malloc() (specific to json.h)
* No library dependancies.
* Only need to include a single file (emJSON.h or json.h)

Examples
--------

### emJSON.h:
``` C
json_t test = emJSON_init();
emJSON_insert(&test, "entry_1", "JSON");
emJSON_insert(&test, "entry_2", "Is");
emJSON_insert(&test, "entry_3", "Cool");

printf("%s\n", emJSON_get(&test, "entry_1"));
printf("%s\n", emJSON_get(&test, "entry_2"));
printf("%s\n", emJSON_get(&test, "entry_3"));

char *str = emJSON_string(&test);
printf("%s\n", str);
free(str);
emJSON_free(&test);

/*
JSON
Is
Cool
{"entry_2":"Is","entry_3":"Cool","entry_1":"JSON"}
*/
```

### json.h:
``` C
char buf[80];
json_entry_t table[8];

json_t test = json_init(buf, 80, table, 8);
json_insert(&test, "entry_1", "JSON");
json_insert(&test, "entry_2", "Is");
json_insert(&test, "entry_3", "Cool");

printf("%s\n", json_get(&test, "entry_1"));
printf("%s\n", json_get(&test, "entry_2"));
printf("%s\n", json_get(&test, "entry_3"));

char str[50];
json_strcpy(str, &test);
printf("%s\n", str);

/*
JSON
Is
Cool
{"entry_2":"Is","entry_3":"Cool","entry_1":"JSON"}
*/
```

Implementation
--------------

The algorithm is highly inspired by [Python's Dictionary implementation](http://svn.python.org/projects/python/trunk/Objects/dictobject.c) and [Python's string hash algorithm](https://svn.python.org/projects/python/trunk/Objects/stringobject.c).


TODOs
--------

* [ ] JSON encoding
* [x] JSON decoding
* [x] Support String type
* [ ] Support Integer type
* [ ] Support Number type (floating point)
* [ ] Support Boolean literals (ture, false)
* [ ] Support Null literal (null)
* [ ] Support array type
* [ ] Support object type

Limitations
-----------

### Common
* The format of JSON should be correct. Otherwise, the code will break.
* No comment in JSON allowed.


### json.h specific

Because it does not use dynamic memory allocations there are serveral limitation:

* buffer should be big enough that it does not have overflow.
* json entry size is not auto-resizable. Therefore use big enough json entry table, or `json_replace_buffer()` to move to a bigger buffer.
* The json entry size SHOULD be a power of 2. (e.g. 2, 4, 8, 16...)
* Buffer size of string value is a multiple of 8. If you change string value over than string buffer size, run `json_remove()` then `json_insert()`.