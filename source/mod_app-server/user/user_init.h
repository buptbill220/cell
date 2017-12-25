#ifndef _USER_INIT_H_
#define _USER_INIT_H_

#include "app_common/global.h"

#ifdef __cplusplus
extern "C" {
#endif

 ngx_int_t set_init_mysql_query_result(ngx_http_request_t* r, ngx_str_t* data);
 ngx_int_t get_init_mysql_query_status(ngx_http_request_t* r, ngx_str_t* data);
 ngx_int_t set_init_mysql_insert_result(ngx_http_request_t* r, ngx_str_t* data);
 ngx_int_t get_init_resp(ngx_http_request_t* r, ngx_str_t* data);

#ifdef __cplusplus
}
#endif

#endif
