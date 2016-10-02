emJSON - JSON for embedded systems
==================================

_emJSON is a fast and memory efficient JSON library for embedded systems, written in C99_

There are two libraries in this project:
* `json.h`: Low-level JSON library that does not use dynamic memory allocation (e.g. malloc().)
* `emJSON.h` : Easy-to-use JSON library that is built open json.h functions and makes use of dynamic memory allocation.

Features
--------

* JSON Encoding.
* JSON Decoding.
* Easy to use.
* Fast and efficient hash map algorithm.
* Minimized use of memory and memory fragmentation.
* No use of malloc() (json.h only)
* No extra library dependencies. (Only C standard libraries used.)
* Only need to include a single file (emJSON.h or json.h)

Simple Examples
---------------

### json.h:
``` C
char str_input[] = "{\"sensor1\":0.045600,\"message\":\"JSON Is Cool\",\"sensor2\":142}";

char buf[256];
json_t test = json_init(buf, 256, 4);  // 4 is table size, this MUST be power of 2.
json_parse(&test, str_input);

float sensor1 = json_get_float(&test, "sensor1"); // 0.0456
int   sensor2 = json_get_int(&test, "sensor2");   // 142
char *message = json_get_str(&test, "message");   // "JSON Is Cool

json_insert_int(&test, "intInput", 999); // insert

char str[80];
json_strcpy(str, &test);
printf("%s\n", str);
// {"sensor1":0.045600,"intInput":999,"message":"JSON Is Cool","sensor2":142}
```

### emJSON.h:
``` C
char str_input[] = "{\"sensor1\":0.045600,\"message\":\"JSON Is Cool\",\"sensor2\":142}";

json_t test = emJSON_init();  // 8 is table size
emJSON_parse(&test, str_input);

float sensor1 = emJSON_get_float(&test, "sensor1"); // 0.0456
int   sensor2 = emJSON_get_int(&test, "sensor2");   // 142
char *message = emJSON_get_str(&test, "message");   // "JSON Is Cool

emJSON_insert_int(&test, "intInput", 999); // insert

char *str = emJSON_string(&test);
printf("%s\n", str);
// {"sensor1":0.045600,"intInput":999,"message":"JSON Is Cool","sensor2":142}

free(str);
emJSON_free(&test);
```
Implementation
--------------

The algorithm is highly inspired by [Python's Dictionary implementation](http://svn.python.org/projects/python/trunk/Objects/dictobject.c) and [Python's string hash algorithm](https://svn.python.org/projects/python/trunk/Objects/stringobject.c).


TODOs
--------

* [x] JSON encoding
* [x] JSON decoding
* [ ] No sprintf(), sscanf().
* [ ] Merge functions
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