#ifndef _USER_CHECK_REG_H_
#define _USER_CHECK_REG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "app_common/global.h"

ngx_int_t set_check_user_reg_result(ngx_http_request_t* r, ngx_str_t* data);
ngx_int_t get_check_user_reg_resp(ngx_http_request_t* r, ngx_str_t* data);

#ifdef __cplusplus
}
#endif

#endif
