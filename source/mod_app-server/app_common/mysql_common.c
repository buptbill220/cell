#include "app_common/mysql_common.h"
#include <stdio.h>

unsigned int mysql_safe_escape_string(char *to, unsigned int max_to_size, const char *from, unsigned int from_size)
{
    unsigned int to_size= 0;
    char newchar;
    const char *end;
    
    for (end= from + from_size; from < end; from++) {
        newchar= 0;
        /* All multi-byte UTF8 characters have the high bit set for all bytes. */
        if (!(*from & 0x80)) {
            switch (*from) {
                case 0:
                    newchar= '0';
                    break;
                case '\n':
                    newchar= 'n';
                    break;
                case '\r':
                    newchar= 'r';
                    break;
                case '\032':
                    newchar= 'Z';
                    break;
                case '\\':
                    newchar= '\\';
                    break;
                case '\'':
                    newchar= '\'';
                    break;
                case '"':
                    newchar= '"';
                    break;
                default:
                    break;
            }
        }
        if (newchar != '\0') {
            if ((unsigned int)to_size + 2 > max_to_size) {
                return -1;
            }
            *to++= '\\';
            *to++= newchar;
            to_size++;
        } else {
            if ((unsigned int)to_size + 1 > max_to_size) {
                return -1;
            }
            *to++= *from;
            to_size++;
        }
    }
    *to= 0;
    return to_size;
}

unsigned int mysql_escape_string(char *to, const char *from, unsigned int from_size)
{
    return (unsigned int) mysql_safe_escape_string(to, (from_size * 2), from, from_size);
}

unsigned int mysql_case_null_field(char* to, const char* field, const char* default_value, unsigned int max_size)
{
    unsigned int to_size = 0;
    to_size = snprintf(to, max_size, "(case when %s is NULL then %s else %s end) as %s", field, default_value, field, field);
    return to_size;
}
