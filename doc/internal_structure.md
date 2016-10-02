emJSON sturctures
=================

emJSON objects (`json_t`) consist of the following blocks in the single buffer.
Although this structure is hide from the header file (`json.h`), you can see 
this structure in `json.c`.

```
offset		Block name

  0x00 ----------------------
       |                    |
       |   Header Block     |
       |                    |
       ----------------------
       |                    |
       |    Entry Table     |
       |                    |
       ----------------------
       |                    |
       |      Content       |
       |                    |
       ~~~~~~~~~~~~~~~~~~~~~~
  	   ...
```

### Header Block

Header block is `_header_t` in the source file. The header block is the following:

``` C
typedef struct
{
    int buf_size;
    size_t  buf_idx;
    size_t  table_size;
    int entry_count;
}_header_t;
```

### Entry table block

The entry table is an array of `_entry_t` structure. Each entries in the table
consists of following:

``` C
typedef struct
{
    int32_t hash;
    void *value_ptr;
    char *key;
    json_value_size_t value_size;
    json_value_t value_type;
}_entry_t;
```


### Content block

Names and values of json object are stored in the content block.
