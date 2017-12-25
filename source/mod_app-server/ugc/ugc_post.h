#ifndef _UGC_POST_H_
#define _UGC_POST_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "app_common/global.h"

ngx_int_t set_post_ugc_mysql_result(ngx_http_request_t* r, ngx_str_t* data);
ngx_int_t get_post_ugc_mysql_result_status(ngx_http_request_t* r, ngx_str_t* data);
ngx_int_t get_post_ugc_type(ngx_http_request_t* r, ngx_str_t* data);
ngx_int_t set_post_ugc_select_achieve_type(ngx_http_request_t* r, ngx_str_t* data);
ngx_int_t set_post_ugc_select_achieve_result(ngx_http_request_t* r, ngx_str_t* data);
ngx_int_t get_post_ugc_select_achieve_result_status(ngx_http_request_t* r, ngx_str_t* data);
ngx_int_t set_post_ugc_update_achieve_type(ngx_http_request_t* r, ngx_str_t* data);
ngx_int_t set_post_ugc_update_achieve_result(ngx_http_request_t* r, ngx_str_t* data);
ngx_int_t get_post_ugc_seed_is_local(ngx_http_request_t* r, ngx_str_t* data);
ngx_int_t set_post_ugc_user_achieve_type(ngx_http_request_t* r, ngx_str_t* data);
ngx_int_t set_post_ugc_user_achieve_result(ngx_http_request_t* r, ngx_str_t* data);
ngx_int_t get_post_ugc_user_achieve_result_status(ngx_http_request_t* r, ngx_str_t* data);
ngx_int_t get_post_ugc_resp(ngx_http_request_t* r, ngx_str_t* data);

#ifdef __cplusplus
}
#endif

#endif
