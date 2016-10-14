#include "json.h"
#include <string.h>
#include "json_internal.h"

/*
 * Parser-related structs and functions
 */
struct parser_result_
{
    char *i;
    char *j;
    json_type_t result_type;
};

static struct parser_result_ check_string_(char *input);
static struct parser_result_ check_number_(char *input);

static int insert_(json_t *obj, struct parser_result_ *key, struct parser_result_ *value);

static int is_ws_(char input);
static inline int is_digit_(char input);

/*
 *  Converter-related structs and functions
 */
struct atox_ret_
{
	union
	{
	    float f;
		int i;
	} value;
    uint8_t str_len;
};

static struct atox_ret_ atoi_(const char *str);
static struct atox_ret_ atof_(const char *str);
static int itoa_(int input, char *str, int base);
static int ftoa_(float input, char *str);

/*******************************************************************************
 * Parser functions
 ******************************************************************************/

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
    struct parser_result_ result_name;
    struct parser_result_ result_value;
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
            else if (*i == '{')
            {
            	result_value.i = i;
            	result_value.result_type = JSON_OBJECT;
            }
            else
            {
                return JSON_ERROR;
            }
            // put them into the object
            ret = insert_(obj, &result_name, &result_value);
            // update i
            i = result_value.j;
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
    // skip '}'
    i += 1;
    return (i - input);
}

static struct parser_result_ check_string_(char *input)
{
    struct parser_result_ ret = {
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

static struct parser_result_ check_number_(char *input)
{
    struct parser_result_ ret = {
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
    int idx = 0;
    memset(dest + idx, '{', 1);
    idx += 1;
    // start
    for (size_t i = 0; i < table_size_(obj); i++)
    {
        struct entry_ *entry = table_ptr_(obj) + i;
        if (NULL == entry->key)
        {    // empty entry
            continue;
        }
        // copy key: "<key>":
        memset(dest + idx, '\"', 1);
        idx += 1;
        int str_len = strlen(entry->key);
        strcpy(dest + idx, entry->key);
        strcpy(dest + idx + str_len, "\":");
        idx += str_len + 2;
        // copy value: "<value>",
        switch (entry->value_type)
        {
        case JSON_INT:
        	str_len = itoa_(*(int *)entry->value_ptr, dest + idx, 10);
            break;
        case JSON_FLOAT:
#ifdef __CC_ARM
        	// For the same problem in json_get_float();
        	// The VLDR instruction causes hard fault....
        	{
        		float value;
        		memcpy(&value, entry->value_ptr, sizeof(float));
        		ftoa_(value, dest + idx);
        		break;
        	}
#else	// __GNUC__
        	str_len = ftoa_(*(float *)entry->value_ptr, dest + idx);
            break;
#endif
        case JSON_STRING:
            memset(dest + idx, '\"', 1);
            idx += 1;
            strcpy(dest + idx, (char *)entry->value_ptr);
            str_len = strlen(dest + idx);
            break;
        case JSON_OBJECT:
        	{
        		json_t tmp = {
        				.buf = entry->value_ptr
        		};
        		str_len = json_strcpy(dest + idx, &tmp);
        	}
        	break;
        case JSON_NULL:
            strcpy(dest + idx, "null");
            str_len = 4;
        	break;
        default:
            return JSON_ERROR;
        }
        strcpy(dest + idx + str_len, (JSON_STRING == entry->value_type) ? "\"," : ",");
        idx += str_len + ((JSON_STRING == entry->value_type)? 2 : 1);
    }
    // it's the end
    // -1 is to remove the last ','.
    strcpy(dest + idx -1, "}");
    return idx;
}

int json_strlen(json_t *obj)
{
    char str_buf[30];   // FIXME: Take more string length
    int idx = 0;
    idx += 1;    // '{'
    // start
    for (size_t i = 0; i < table_size_(obj); i++)
    {
        struct entry_ *entry = table_ptr_(obj) + i;
        if (NULL == entry->key)
        {    // empty entry
            continue;
        }
        // "<key>":
        idx += 1; // '\"'
        int str_len = strlen(entry->key);
        idx += str_len + 2;    // <key>":
        // "<value>",
        switch (entry->value_type)
        {
        case JSON_INT:
            str_len = itoa_(*(int *)entry->value_ptr, str_buf, 10);
            break;
        case JSON_FLOAT:
#ifdef __CC_ARM
        	// For the same problem in json_get_float();
        	// The VLDR instruction causes hard fault....
        	{
        		float value;
        		memcpy(&value, entry->value_ptr, sizeof(float));
        		str_len = ftoa_(value, str_buf);
        		break;
        	}
#else	// __GNUC__
        	str_len = ftoa_(*(float *)entry->value_ptr, str_buf);
            break;
#endif
        case JSON_STRING:
            idx += 1;    // '\"'
            str_len = strlen((char *)entry->value_ptr);
            break;
        case JSON_OBJECT:
        	{
        		json_t tmp = {
        				.buf = entry->value_ptr
        		};
        		str_len = json_strlen(&tmp);
        	}
        	break;
        case JSON_NULL:
        	str_len = 4; // "null"
        	break;
        default:
            return JSON_ERROR;
        }
        idx += str_len + ((JSON_STRING == entry->value_type) ? 2 : 1); // <value>",
    }
    // it's the end, '}'
    // consider removing the last ','.
    return idx;
}

static int insert_(json_t *obj, struct parser_result_ *key, struct parser_result_ *value)
{
    // store the end character then delete
    char name_end = *key->j;
    char value_end = *value->j;
    *key->j = '\0';
    if (JSON_OBJECT != value->result_type)
    {
        *value->j = '\0';
    }
    // convert value.
    union {
    	int32_t i;
    	float f;
    	char *str;
    	json_t obj;
    } input;
    int ret;
    switch (value->result_type)
    {
    case JSON_STRING:
        input.str = value->i;
        ret = json_insert(obj, key->i, input.str, value->result_type);
        break;
    case JSON_INT:
        input.i = atoi_(value->i).value.i;
        ret = json_insert(obj, key->i, &input.i, value->result_type);
        break;
    case JSON_FLOAT:
        input.f = atof_(value->i).value.f;
        ret = json_insert(obj, key->i, &input.f, value->result_type);
        break;
    case JSON_OBJECT:
    	// FIXME: Find a better way
    	/*
    	 * Object insertion strategy:
    	 * 1. Create a temporary object that takes over the rest of buffer.
    	 * 2. Then insert.
    	 * 3. Finally trim the buffer size of the child and move the index back.
    	 */
    	// Make NULL type insertion first?
    	// what about inserting null object?
        ret = json_insert_empty_obj(obj, key->i, buf_size_(obj)-(buf_idx_(obj)+strlen(key->i)+1));
        input.obj = json_get_obj(obj, key->i);
        if (NULL == input.obj.buf)
        {
        	return JSON_BUFFER_FULL;
        }
        value->j = value->i;
        value->j += json_parse(&input.obj, value->i);
        if (value->j < value->i)
        {
        	return (value->j < value->i); // convert into an error code.
        }
        // finally trim down the buffer
        {
        	size_t offset = buf_size_(&input.obj) - buf_idx_(&input.obj);
        	buf_size_(&input.obj) -= offset;
        	buf_idx_(obj) -= offset;
        	struct entry_ *tmp_entry = table_ptr_(obj) + idx_in_parent_(&input.obj);
        	tmp_entry->value_size -= offset;
        }
        break;
    default:
        return JSON_ERROR;
    }
    // restore charters
    *key->j = name_end;

    if (JSON_OBJECT != value->result_type)
    {
        *value->j = value_end;
    }
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

/*******************************************************************************
 * xtox funtions
 ******************************************************************************/

struct atox_ret_ atoi_(const char *str)
{
    struct atox_ret_ ret;
    int ret_int = 0;
    int8_t sign = 1;
    unsigned int len = 0;
    if (NULL == str)
    {
        return (struct atox_ret_){0};
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
    ret = (struct atox_ret_){
        .value.i = ret_int * sign,
        .str_len = len
    };
    return ret;
}

struct atox_ret_ atof_(const char *str)
{
    struct atox_ret_ ret;
    struct atox_ret_ tmp;
    uint8_t is_plus = 1;
    int int_part = 0;    // integer part
    float frac_part = 0;    // fraction part
    size_t frac_len = 0;// length of fraction part
    int expo_part = 0;    // exponent part

    unsigned int len = 0;
    if (NULL == str)
    {
        return (struct atox_ret_){0};
    }
    // get integer part
    if (*str == '-')
    {
        is_plus = 0;
        str += 1;
        len += 1;
    }
    tmp = atoi_(str);
    int_part = tmp.value.i;

    str += tmp.str_len;
    len += tmp.str_len;

    // get fraction part check
    if(*str == '.')
    {
        str += 1;
        tmp = atoi_(str);
        frac_part = (float) tmp.value.i;
        frac_len = tmp.str_len;

        str += tmp.str_len;
        len += tmp.str_len + 1;
    }

    // exponent part
    if (*str == 'e' ||
        *str == 'E')
    {
        str += 1;
        tmp = atoi_(str);
        expo_part = tmp.value.i;

        str += tmp.str_len;
        len += tmp.str_len + 1;
    }

    // put them together
    ret.value.f = (float) int_part;
    if (frac_len > 0)
    {
        // add fraction
        int frac_divisor = 1;
        for(; frac_len > 0; frac_len--)
        {
            frac_divisor *= 10;
        }
        frac_part /= frac_divisor;
        if (ret.value.f < 0)
        {
            ret.value.f *= -1;
            ret.value.f += frac_part;
            ret.value.f *= -1;
        }
        else
        {
            ret.value.f += frac_part;
        }
    }

    if (expo_part != 0)
    {
        // multiply exponent
        int expo_multiplier = 1;
        int8_t expo_signed = 1;
        if (expo_part < 0)
        {
            expo_part *= -1;
            expo_signed = 0;
        }
        for(; expo_part > 0; expo_part--)
        {
            expo_multiplier *= 10;
        }
        if (expo_signed)
        {
            ret.value.f *= expo_multiplier;
        }
        else
        {
            ret.value.f /= expo_multiplier;
        }
    }

    if (!is_plus)
    {
        ret.value.f *= -1;
    }
    // done
    ret.str_len = len;
    return ret;
}

static int itoa_(int input, char *str, int base)
{
    unsigned int n = 0;
    int d = 1;
    if (input < 0)
    {
        *str++ = '-';
        input *= -1;
        n++;
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

static int ftoa_(float value, char* str)
{
    union
    {
        float f;
        struct
        {
            uint16_t    mantissa_lo : 16;
            uint16_t    mantissa_hi : 7;
            uint16_t    exponent : 8;
            uint8_t     sign : 1;
        } bits;
    } input;
    uint32_t significand;
    uint32_t int_part;
    uint32_t frac_part;
    int16_t exponent;
    char *p;


    if (value == (float)0.0)
    {
        strcpy(str, "0,0");
        return 1;
    }

    input.f = value;
    significand = input.bits.mantissa_lo;
    significand += ((uint32_t) input.bits.mantissa_hi << 16);
    //add the 24th bit to get 1.mmmm format
    significand += 0x00800000;
    exponent = input.bits.exponent - 127;
    int_part = 0;

    // TODO: include exponent notation (E, e) in the string.
    if (exponent >= 31)
    {
        strcpy(str, "Inf");
        return 3;
    }
    else if (exponent < -23)
    {
        strcpy(str, "0");
        return 1;
    }
    else if (exponent >= 23)
    {    // significand bits are 23+1 bits so it need to be moved to show integer parts well. And it won't have faction part.
        int_part = significand << (exponent - 23);
        frac_part = 0;
    }
    else if (exponent >= 0)
    {
        int_part = significand >> (23 - exponent);
        frac_part = (significand << (exponent + 1)) & 0xFFFFFF;
    }
    else /* if (exponent < 0) */
    {
        frac_part = (significand & 0xFFFFFF) >> (-(exponent + 1));
    }

    p = str;

    if (input.bits.sign)
    {
        *p++ = '-';
    }
    if (int_part == 0)
    {
        *p++ = '0';
    }
    else
    {
        itoa_(int_part, p, 10);
        while (*p)
        {
            p++;
        }
    }
    *p++ = '.';

    if (frac_part == 0)
    {
        *p++ = '0';
    }
    else
    {
        uint8_t i;
        // Here 5 is precision.
        for (i = 0; i < 5; i++)
        {
            /* frac_part *= 10;    */
            frac_part = (frac_part << 3) + (frac_part << 1);

            *p++ = (frac_part >> 24) + '0';
            frac_part &= 0xFFFFFF;
        }
        /* delete ending zeroes */
        for (--p; p[0] == '0' && p[-1] != '.'; --p)
        {
        }
        ++p;
    }
    *p = '\0';

    return strlen(str);
}

