#ifndef _UGC_PULL_UGC_LIST_H_
#define _UGC_PULL_UGC_LIST_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "app_common/global.h"

ngx_int_t set_pull_ugc_list_type(ngx_http_request_t* r, ngx_str_t* data);
ngx_int_t set_pull_ugc_list_mysql_result(ngx_http_request_t* r, ngx_str_t* data);
ngx_int_t get_pull_ugc_pull_type(ngx_http_request_t* r, ngx_str_t* data);
ngx_int_t get_pull_ugc_list_has_ugc(ngx_http_request_t* r, ngx_str_t* data);
ngx_int_t get_pull_ugc_list_resp(ngx_http_request_t* r, ngx_str_t* data);

#ifdef __cplusplus
}
#endif

#endif
