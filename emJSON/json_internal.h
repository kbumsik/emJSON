/*
 * json_internal.h
 *
 *  Created on: Oct 6, 2016
 *      Author: Bumsik Kim
 */

#ifndef JSON_INTERNAL_H_
#define JSON_INTERNAL_H_

struct entry_
{
    int32_t hash;
    void *value_ptr;
    char *key;
    uint8_t value_size;
    json_type_t value_type;
};

struct header_
{
    size_t buf_size;
    size_t buf_idx;
    size_t table_size;
    size_t entry_count;
};


// pointer macros
#define header_ptr_(obj)  ((struct header_ *)((obj)->buf))
#define table_ptr_(obj)  ((struct entry_ *)((obj)->buf + sizeof(struct header_)))
#define content_ptr_(obj) ((obj)->buf + sizeof(struct header_) + table_byte_size_(obj))

// pointer size functions
#define buf_size_(obj)  (header_ptr_(obj)->buf_size)

#define buf_idx_(obj)   (header_ptr_(obj)->buf_idx)

#define table_size_(obj)    (header_ptr_(obj)->table_size)

#define entry_count_(obj)   (header_ptr_(obj)->entry_count)

static inline size_t table_byte_size_(json_t *obj)
{
    return header_ptr_(obj)->table_size * sizeof(struct entry_);
}

static inline size_t content_byte_size_(json_t *obj)
{
    return header_ptr_(obj)->buf_size -
        (sizeof(struct header_) + header_ptr_(obj)->table_size * sizeof(struct entry_));
}

#endif /* JSON_INTERNAL_H_ */

