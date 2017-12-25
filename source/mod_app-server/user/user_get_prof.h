#ifndef _USER_GET_PROF_H_
#define _USER_GET_PROF_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "app_common/global.h"

ngx_int_t set_get_user_prof_mysql_result(ngx_http_request_t* r, ngx_str_t* data);
ngx_int_t get_get_user_prof_resp(ngx_http_request_t* r, ngx_str_t* data);

#ifdef __cplusplus
}
#endif

#endif
