emJSON - JSON for embedded system
=================================

_emJSON is a fast and memory efficient JSON library for embedded systems, written in C99_

There are two libraries in this project:
* `json_*`: Low-level JSON library that does not use dynamic memory allocation (e.g. malloc().)
* `emJSON_*` : Easy-to-use JSON library that is built open `json_*` functions and make use of dynamic memory allocation.

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


Example
-------

### `emJSON_*`:
``` C
TBD
```

### `json_*`:
``` C
TBD
```

Implementation
--------------

The algorithm is highly inspired my [Python's Dictionary implementation](http://svn.python.org/projects/python/trunk/Objects/dictobject.c) and [Python's string hash algorithm](http://svn.python.org/projects/python/trunk/Objects/dictobject.c).