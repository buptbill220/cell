/**
 *  Copyright (c) 2015 cell-app.com
 *  All rights reserved.
 *
 *  author: fangming
 *  date: 2015.02.09
 *  
 */

#ifndef _COMMON_FUNC_H_
#define _COMMON_FUNC_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "app_common/global.h"

ngx_variable_value_t* get_var_ptr(ngx_http_request_t* r, ngx_int_t idx);
session_t* get_session_ptr(ngx_http_request_t* r);
ngx_int_t set_message_type(ngx_http_request_t* r, ngx_str_t data);
ngx_int_t set_message_data(ngx_http_request_t* r, ngx_str_t data);
ngx_int_t set_mysql_query_string(ngx_http_request_t* r, ngx_str_t data);
ngx_int_t set_redis_query_string(ngx_http_request_t* r, ngx_str_t data);
ngx_int_t set_check_session(ngx_http_request_t* r, ngx_str_t data);

ngx_int_t get_session_id(ngx_http_request_t* t, session_t* s);
ngx_int_t build_session(ngx_http_request_t* r, ngx_str_t* data);
ngx_int_t set_message(ngx_http_request_t* r, ngx_str_t* data);
ngx_int_t set_user_session(ngx_http_request_t* r, ngx_str_t* data);
ngx_int_t get_pvlog(ngx_http_request_t* r, ngx_str_t* data);

#ifdef __cplusplus
}
#endif

#endif
