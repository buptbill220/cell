#ifndef _MYSQL_COMMON_H_
#define _MYSQL_COMMON_H_

#ifdef __cplusplus
extern "C" {
#endif

unsigned int mysql_safe_escape_string(char* to,unsigned int max_to_size, const char* from, unsigned int from_size);
unsigned int mysql_escape_string(char* to, const char* from, unsigned int from_size);
unsigned int mysql_case_null_field(char* to, const char* field, const char* default_value, unsigned int max_size);

#ifdef __cplusplus
}
#endif

#endif
