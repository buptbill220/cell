#ifndef _UGC_GET_UGC_H_
#define _UGC_GET_UGC_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "app_common/global.h"

ngx_int_t set_get_ugc_mysql_result(ngx_http_request_t* r, ngx_str_t* data);
ngx_int_t get_get_ugc_resp(ngx_http_request_t* r, ngx_str_t* data);

#ifdef __cplusplus
}
#endif

#endif
