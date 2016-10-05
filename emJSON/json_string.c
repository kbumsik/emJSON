#include "json.h"
#include <string.h>
#include <stdio.h>

typedef struct
{
    char *i;
    char *j;
    json_value_t result_type;
}parser_result_t_;

static parser_result_t_ check_string_(char *input);
static parser_result_t_ check_number_(char *input);

static int insert_(json_t *obj, parser_result_t_ *key, parser_result_t_ *value);

static int is_ws_(char input);
static inline int is_digit_(char input);


#include <stddef.h>

// internals
struct atox_ret_t_
{
	int ret_int;
	float ret_float;
	uint8_t len;
};

static struct atox_ret_t_ atoi_(const char *str);
static int itoa_(int input, char *str, int base);

typedef struct
{
    int32_t hash;
    void *value_ptr;
    char *key;
    json_value_size_t value_size;
    json_value_t value_type;
}entry_t_;

typedef struct
{
    size_t buf_size;
    size_t buf_idx;
    size_t table_size;
    size_t entry_count;
}header_t_;

// pointer macros
#define header_ptr_(obj)  ((header_t_ *)((obj)->buf))
#define table_ptr_(obj)  ((entry_t_ *)((obj)->buf + sizeof(header_t_)))
#define table_size_(obj)    (header_ptr_(obj)->table_size)

// converting from string functions

int json_parse(json_t *obj, char *input)
{
    enum
    {
        start, end, value, name
    }state;
    char *i = input;
    if (strlen(input) < 2)
    {
        return JSON_ERROR;
    }
    // skip whitespace
    while (is_ws_(*i))
    {
        i += 1;
    }
    if (*i != '{')
    {
        return JSON_ERROR;
    }
    i += 1;
    state = start;
    parser_result_t_ result_name;
    parser_result_t_ result_value;
    int ret;
    while (end != state)
    {
        switch (state)
        {
        case start:
            while (is_ws_(*i))
            {
                i += 1;
            }
            if (*i == '}')
            {
                state = end;
                continue;
            }
            else if (*i == '"')
            {
                state = name;
                continue;
            }
            else
            {
                return JSON_ERROR;
            }
            break;
        case name:
            while (is_ws_(*i))
            {
                i += 1;
            }
            if (*i == '"')
            {
                result_name = check_string_(i);
            }
            else
            {
                return JSON_ERROR;
            }
            // update i
            i = result_name.j;
            // Check '"'
            if (*i != '"')
            {
                return JSON_ERROR;
            }
            i += 1;
            // reach to ':'
            while (is_ws_(*i))
            {
                i += 1;
            }
            if(*i == ':')
            {
                i += 1;
                state = value;
                continue;
            }
            else
            {
                return JSON_ERROR;
            }
            break;
        case value:
            while (is_ws_(*i))
            {
                i += 1;
            }
            if (*i == '"')
            {
                result_value = check_string_(i);
            }
            else if (*i == '-' || is_digit_(*i))
            {
                result_value = check_number_(i);
            }
            else
            {
                return JSON_ERROR;
            }
            // update i
            i = result_value.j;
            // put them into the object
            ret = insert_(obj, &result_name, &result_value);
            if (JSON_OK != ret)
            {
                return ret;
            }
            // Then keep going
            if (result_value.result_type == JSON_STRING &&
                *i == '"')
            {
                i += 1;
            }
            while (is_ws_(*i))
            {
                i += 1;
            }
            if (*i == ',')
            {
                i += 1;
                state = name;
                continue;
            }
            else if (*i == '}')
            {
                state = end;
                continue;
            }
            else
            {
                return JSON_ERROR;
            }
            break;
        default:
            return JSON_ERROR;
        }
    }
    return JSON_OK;
}

static parser_result_t_ check_string_(char *input)
{
    parser_result_t_ ret = { 
        .i = NULL,
        .j = NULL,
        .result_type = JSON_UNKNOWN
    };
    ret.i = input;
    ret.j = input;
    while (is_ws_(*ret.i))
    {
        ret.i += 1; ret.j += 1;
    }
    if (*ret.i != '"')
    {
        ret.result_type = JSON_UNKNOWN;
        return ret;
    }
    ret.i += 1; ret.j += 1;
    // from now i is fixed.
    // TODO: Detect also special characters (such as \")
    while (*ret.j != '"')
    {
        ret.j += 1;
    }
    // Done. Success
    ret.result_type = JSON_STRING;
    return ret;
}

static parser_result_t_ check_number_(char *input)
{
    parser_result_t_ ret = { 
        .i = NULL,
        .j = NULL,
        .result_type = JSON_UNKNOWN
    };
    enum
    {
        integer,
        decimal,
        end
    }state;
    ret.i = input;
    ret.j = input;
    
    // skip whitespace
    while (is_ws_(*ret.i))
    {
        ret.i += 1; ret.j += 1;
    }
    if (!(*ret.i == '-' || is_digit_(*ret.i)))
    {
        ret.result_type = JSON_UNKNOWN;
        return ret;
    }
    if (*ret.i == '0')
    {
        state = decimal;
    }
    else
    {
        state = integer;
    }
    // from now i is fixed.
    
    while (state != end)
    {
        switch (state)
        {
        case integer:
        	if (*ret.j == '-')
        	{
        		ret.j += 1;
        	}
            while (is_digit_(*ret.j))
            {
                ret.j += 1;
            }
            if (*ret.j == '.' ||
                *ret.j == 'e' ||
                *ret.j == 'E')
            {
                state = decimal;
                continue;
            }
            ret.result_type = JSON_INT;
            state = end;
            break;
        case decimal:
        	if (*ret.j == '-')
        	{
        		ret.j += 1;
        	}
            while (is_digit_(*ret.j))
            {
                ret.j += 1;
            }
            // decimal point
            if (*ret.j == '.')
            {
                ret.j += 1;
            }
            // mantissa
            while (is_digit_(*ret.j))
            {
                ret.j += 1;
            }
            // Exponent
            if (*ret.j == 'e' ||
                *ret.j == 'E')
            {
                // Do noting yet.
                ret.j += 1;
                if (*ret.j == '-')
                {
                    // Do noting yet.
                    ret.j += 1;
                }
            }
            while (is_digit_(*ret.j))
            {
                ret.j += 1;
            }
            ret.result_type = JSON_FLOAT;
            state = end;
            break;
        default:
            ret.result_type = JSON_UNKNOWN;
            return ret;
        }
    }
    // convert
    return ret;
}

/*******************************************************************************
 * String-building functions
 ******************************************************************************/

int json_strcpy(char *dest, json_t *obj)
{
    char str_buf[30];   // FIXME: Take more string length
    int idx = 0;
    memset(dest + idx, '{', 1);
    idx += 1;
    // start
    for (size_t i = 0; i < table_size_(obj); i++)
    {
        entry_t_ entry = table_ptr_(obj)[i];
        if (NULL == entry.key)
        {	// empty entry
            continue;
        }
        // copy key: "<key>":
        memset(dest + idx, '\"', 1);
        idx += 1;
        int str_len = strlen(entry.key);
        strcpy(dest + idx, entry.key);
        strcpy(dest + idx + str_len, "\":");
        idx += str_len + 2;
        // copy value: "<value>",
        switch (entry.value_type)
        {
        case JSON_INT:
        	itoa_(*(int *)entry.value_ptr, str_buf, 10);
            break;
        case JSON_FLOAT:
            sprintf(str_buf, "%f", *(float *)entry.value_ptr);
            break;
        case JSON_STRING:
            memset(dest + idx, '\"', 1);
            idx += 1;
            sprintf(str_buf, "%s", (char *)entry.value_ptr);
            break;
        default:
            return JSON_ERROR;
        }
        str_len = strlen(str_buf);
        strcpy(dest + idx, str_buf);
        strcpy(dest + idx + str_len, (JSON_STRING == entry.value_type) ? "\"," : ",");
        idx += str_len + ((JSON_STRING == entry.value_type)? 2 : 1);
    }
    // it's the end
    // -1 is to remove the last ','.
    strcpy(dest + idx -1, "}");
    return JSON_OK;
}

size_t json_strlen(json_t *obj)
{
    char str_buf[30];   // FIXME: Take more string length
    int idx = 0;
    idx += 1;	// '{'
    // start
    for (size_t i = 0; i < table_size_(obj); i++)
    {
        entry_t_ entry = table_ptr_(obj)[i];
        if (NULL == entry.key)
        {	// empty entry
            continue;
        }
        // "<key>":
        idx += 1; // '\"'
        int str_len = strlen(entry.key);
        idx += str_len + 2;	// <key>":
        // "<value>",
        switch (entry.value_type)
        {
        case JSON_INT:
        	str_len = itoa_(*(int *)entry.value_ptr, str_buf, 10);
            break;
        case JSON_FLOAT:
            sprintf(str_buf, "%f", *(float *)entry.value_ptr);
            str_len = strlen(str_buf);
            break;
        case JSON_STRING:
            idx += 1;	// '\"'
            sprintf(str_buf, "%s", (char *)entry.value_ptr);
            str_len = strlen(str_buf);
            break;
        default:
            return JSON_ERROR;
        }
        idx += str_len + ((JSON_STRING == entry.value_type) ? 2 : 1); // <value>",
    }
    // it's the end, '}'
    // consider removing the last ','.
    return idx;
}

static int insert_(json_t *obj, parser_result_t_ *key, parser_result_t_ *value)
{
    // store the end character then delete
    char name_end = *key->j;
    char value_end = *value->j;
    *key->j = '\0';
    *value->j = '\0';
    // convert value.
    int32_t input = 0;
    void *input_ptr = &input;
    switch (value->result_type)
    {
    case JSON_STRING:
        input_ptr = value->i;
        break;
    case JSON_INT:
    	input = atoi_(value->i).ret_int;
        break;
    case JSON_FLOAT:
        sscanf(value->i, "%f", (float *)input_ptr);
        break;
    default:
        return JSON_ERROR;
    }
    int ret = json_insert(obj, key->i, input_ptr, value->result_type);
    // restore charters
    *key->j = name_end;
    *value->j = value_end;
    return ret;
}


static int is_ws_(char input)
{
    if (input == 0x20 || input == 0x09      // space, horizontal tab
        || input == 0x0a || input == 0x0d)  // LF/NL, CR
    {
        return 1;
    }
    else return 0;
}

static inline int is_digit_(char input)
{
    if (('0' <= input) && (input <= '9'))
    {
        return 1;
    }
    else return 0;
}



// Functions definitions

static int itoa_(int input, char *str, int base)
{
	unsigned int n = 0;
	int d = 1;
	if (input < 0)
	{
		*str++ = '-';
		input *= -1;
		n++;	// TODO: check it putting n++ here is OK
	}
	while((input / d) >= base)
	{
		d *= base;
	}
	while (d != 0)
	{
		int digit = input / d;
		input %= d;
		d /= base;
		if (n || digit > 0 || d == 0)
		{
			*str++ = digit + ((digit < 10)? '0': 'a' - 10);
			n++;
		}
	}
	*str = '\0';
	return n;
}

struct atox_ret_t_ atoi_(const char *str)
{
	struct atox_ret_t_ ret;
	int ret_int = 0;
	int8_t sign = 1;
	unsigned int len = 0;
	if (NULL == str)
	{
		return (struct atox_ret_t_){0};
	}
	if(*str == '-')
	{
		sign = -1;
		str++;
	}
	for(;*str != '\0'; str++)
	{
		if (!(('0' <= *str) && (*str <= '9')))
		{
			break;
		}
		len += 1;
		ret_int = ret_int * 10 + (*str - '0');
	}
	ret = (struct atox_ret_t_){
		.ret_int = ret_int * sign,
		.len = len
	};
	return ret;
}
