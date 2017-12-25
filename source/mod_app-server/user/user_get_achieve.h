#ifndef _USER_GET_ACHIEVE_H_
#define _USER_GET_ACHIEVE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "app_common/global.h"

ngx_int_t get_cache_achievement(ngx_http_request_t* r, ngx_str_t* data);
ngx_int_t set_get_user_achieve_result_r(ngx_http_request_t* r, ngx_str_t* data);
ngx_int_t get_get_user_achieve_result_status_r(ngx_http_request_t* r, ngx_str_t* data);
ngx_int_t get_get_user_achieve_result_m(ngx_http_request_t* r, ngx_str_t* data);
ngx_int_t get_get_user_achieve_resp(ngx_http_request_t* r, ngx_str_t* data);

#ifdef __cplusplus
}
#endif

#endif
