#ifndef _USER_REGISTER_H_
#define _USER_REGISTER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "app_common/global.h"

ngx_int_t set_register_mysql_result(ngx_http_request_t* r, ngx_str_t* data);
ngx_int_t get_register_resp(ngx_http_request_t* r, ngx_str_t* data);

#ifdef __cplusplus
}
#endif

#endif
