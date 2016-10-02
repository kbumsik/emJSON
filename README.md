emJSON - JSON for embedded systems
==================================

_emJSON is a fast and memory efficient JSON library for embedded systems, written in C99_

There are two libraries in this project. You may use either one, depending on your favor:
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

char buffer[256];
json_t test = json_init(buffer, 256, 4);  // 4 is table size, this MUST be power of 2.
json_parse(&test, str_input);

float sensor1 = json_get_float(&test, "sensor1"); // 0.0456
int   sensor2 = json_get_int(&test, "sensor2");   // 142
char *message = json_get_str(&test, "message");   // "JSON Is Cool"

json_insert_int(&test, "intInput", 999); // insert

char str[80];
json_strcpy(str, &test);
printf("%s\n", str);
// {"sensor1":0.045600,"intInput":999,"message":"JSON Is Cool","sensor2":142}
```

### emJSON.h:
``` C
char str_input[] = "{\"sensor1\":0.045600,\"message\":\"JSON Is Cool\",\"sensor2\":142}";

// The difference to json.h is emJSON does not require declaring buffer. 
json_t test = emJSON_init();  // 8 is table size
emJSON_parse(&test, str_input);

float sensor1 = emJSON_get_float(&test, "sensor1"); // 0.0456
int   sensor2 = emJSON_get_int(&test, "sensor2");   // 142
char *message = emJSON_get_str(&test, "message");   // "JSON Is Cool"

emJSON_insert_int(&test, "intInput", 999); // insert

char *str = emJSON_string(&test);
printf("%s\n", str);
// {"sensor1":0.045600,"intInput":999,"message":"JSON Is Cool","sensor2":142}

free(str);
// emJSON.h objects needs to be freed after used instead.
emJSON_free(&test);
```

Supported Devices
-----------------

* x86 and x86-64 based general purpose computers (such as Windows/Linux/Unix computers.)
* 32-bit ARM Cortex-M devices (e.g. STM32F4 Series)
* Arduino and AVR Microcontrollers (Not yet, planned.)

How To Use
----------

All you need is the header and source files in `emJSON` folder.
You may also want to see 
[documentations in doc folder](docs)
before you get started.
For testing and using existing sample codes, please look at 
`examples` folder. 


