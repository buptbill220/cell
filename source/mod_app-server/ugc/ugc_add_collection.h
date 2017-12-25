#ifndef _UGC_ADD_CLT_H_
#define _UGC_ADD_CLT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "app_common/global.h"

ngx_int_t set_query_clt_mysql_result(ngx_http_request_t* r, ngx_str_t* data);
ngx_int_t get_add_collection_status(ngx_http_request_t* r, ngx_str_t* data);
ngx_int_t set_add_clt_mysql_result(ngx_http_request_t* r, ngx_str_t* data);
ngx_int_t get_add_collection_resp(ngx_http_request_t* r, ngx_str_t* data);

#ifdef __cplusplus
}
#endif

#endif
