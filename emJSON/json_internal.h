/*
 * json_internal.h
 *
 *  Created on: Oct 6, 2016
 *      Author: Bumsik Kim
 */

#ifndef JSON_INTERNAL_H_
#define JSON_INTERNAL_H_

struct _entry {
    int32_t hash;
    void *value_ptr;
    char *key;
    size_t value_size;
    json_type_t value_type;
};

struct _header {
	void *parent;
	size_t parent_entry_idx;	// FIXME: deal with it.
    size_t buf_size;
    size_t buf_idx;
    size_t table_size;
    size_t entry_count;
};


// pointer macros
#define _header_ptr(obj)  ((struct _header *)((obj)->buf))
#define _parent_ptr(obj)  (_header_ptr(obj)->parent)
#define _table_ptr(obj)  ((struct _entry *)((obj)->buf + sizeof(struct _header)))
#define _content_ptr(obj) ((obj)->buf + sizeof(struct _header) + _table_byte_size(obj))


// pointer,size, and index functions
#define _idx_in_parent(obj)  (_header_ptr(obj)->parent_entry_idx)

#define _buf_size(obj)  (_header_ptr(obj)->buf_size)

#define _buf_idx(obj)   (_header_ptr(obj)->buf_idx)

#define _table_size(obj)    (_header_ptr(obj)->table_size)

#define _entry_count(obj)   (_header_ptr(obj)->entry_count)

static inline size_t _table_byte_size(json_t *obj)
{
    return _header_ptr(obj)->table_size * sizeof(struct _entry);
}

static inline size_t _content_byte_size(json_t *obj)
{
    return _header_ptr(obj)->buf_size -
        (sizeof(struct _header) + _header_ptr(obj)->table_size * sizeof(struct _entry));
}


#endif /* JSON_INTERNAL_H_ */

