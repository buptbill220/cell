#ifndef _UGC_UPLOAD_H_
#define _UGC_UPLOAD_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "app_common/global.h"

ngx_int_t set_upload_mysql_result(ngx_http_request_t* r, ngx_str_t* data);
ngx_int_t get_upload_mysql_result_status(ngx_http_request_t* r, ngx_str_t* data);
ngx_int_t set_insert_message_achieve_result(ngx_http_request_t* r, ngx_str_t* data);
ngx_int_t get_insert_message_achieve_result_status(ngx_http_request_t* r, ngx_str_t* data);
ngx_int_t set_insert_message_seed_result(ngx_http_request_t* r, ngx_str_t* data);
ngx_int_t get_insert_message_seed_result_status(ngx_http_request_t* r, ngx_str_t* data);
ngx_int_t set_upload_get_user_achieve_result(ngx_http_request_t* r, ngx_str_t* data);
ngx_int_t get_upload_get_user_achieve_result_status(ngx_http_request_t* r, ngx_str_t* data);
ngx_int_t get_upload_resp(ngx_http_request_t* r, ngx_str_t* data);

#ifdef __cplusplus
}
#endif

#endif
