#include "json_parser.h"
#include <string.h>
#include <stdio.h>

typedef struct
{
    char *i;
    char *j;
    json_value_t result_type;
}_parser_result_t;

static _parser_result_t _check_string(char *input);
static _parser_result_t _check_number(char *input);

static int _insert(json_t *obj, _parser_result_t *key, _parser_result_t *value);

static int _is_ws(char input);
static inline int _is_digit(char input);

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
    while (_is_ws(*i))
    {
        i += 1;
    }
    if (*i != '{')
    {
        return JSON_ERROR;
    }
    i += 1;
    state = start;
    _parser_result_t result_name;
    _parser_result_t result_value;
    int ret;
    while (end != state)
    {
        switch (state)
        {
        case start:
            while (_is_ws(*i))
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
            while (_is_ws(*i))
            {
                i += 1;
            }
            if (*i == '"')
            {
                result_name = _check_string(i);
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
            while (_is_ws(*i))
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
            while (_is_ws(*i))
            {
                i += 1;
            }
            if (*i == '"')
            {
                result_value = _check_string(i);
            }
            else if (*i == '-' || _is_digit(*i))
            {
                result_value = _check_number(i);
            }
            else
            {
                return JSON_ERROR;
            }
            // update i
            i = result_value.j;
            // put them into the object
            ret = _insert(obj, &result_name, &result_value);
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
            while (_is_ws(*i))
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

static _parser_result_t _check_string(char *input)
{
    _parser_result_t ret = { 
        .i = NULL,
        .j = NULL,
        .result_type = JSON_UNKNOWN
    };
    ret.i = input;
    ret.j = input;
    while (_is_ws(*ret.i))
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

static _parser_result_t _check_number(char *input)
{
    _parser_result_t ret = { 
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
    while (_is_ws(*ret.i))
    {
        ret.i += 1; ret.j += 1;
    }
    if (!(*ret.i == '-' || _is_digit(*ret.i)))
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
            while (_is_digit(*ret.j))
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
            while (_is_digit(*ret.j))
            {
                ret.j += 1;
            }
            // decimal point
            if (*ret.j == '.')
            {
                ret.j += 1;
            }
            // mantissa
            while (_is_digit(*ret.j))
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
            while (_is_digit(*ret.j))
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

static int _insert(json_t *obj, _parser_result_t *key, _parser_result_t *value)
{
    // store the end character then delete
    char name_end = *key->j;
    char value_end = *value->j;
    *key->j = '\0';
    *value->j = '\0';
    // convert value.
    int32_t dummy = 0;
    void *value_ptr = &dummy;
    switch (value->result_type)
    {
    case JSON_STRING:
        value_ptr = value->i;
        break;
    case JSON_INT:
        sscanf(value->i, "%d", (int *)value_ptr);
        break;
    case JSON_FLOAT:
        sscanf(value->i, "%f", (float *)value_ptr);
        break;
    default:
        return JSON_ERROR;
    }
    int ret = json_insert(obj, key->i, value_ptr, value->result_type);
    // restore charters
    *key->j = name_end;
    *value->j = value_end;
    return ret;
}


static int _is_ws(char input)
{
    if (input == 0x20 || input == 0x09      // space, horizontal tab
        || input == 0x0a || input == 0x0d)  // LF/NL, CR
    {
        return 1;
    }
    else return 0;
}

static inline int _is_digit(char input)
{
    if (('0' <= input) && (input <= '9'))
    {
        return 1;
    }
    else return 0;
}