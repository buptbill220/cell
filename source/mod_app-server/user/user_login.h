#ifndef _USER_LOGIN_H_
#define _USER_LOGIN_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "app_common/global.h"

ngx_int_t set_login_mysql_query_result(ngx_http_request_t* r, ngx_str_t* data);
ngx_int_t get_login_mysql_query_status(ngx_http_request_t* r, ngx_str_t* data);
ngx_int_t get_login_resp(ngx_http_request_t* r, ngx_str_t* data);

#ifdef __cplusplus
}
#endif

#endif
