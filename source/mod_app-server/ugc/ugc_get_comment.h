#ifndef _UGC_GET_COM_H_
#define _UGC_GET_CON_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "app_common/global.h"
    
ngx_int_t set_get_comment_mysql_result(ngx_http_request_t* r, ngx_str_t* data);
ngx_int_t get_get_comment_resp(ngx_http_request_t* r, ngx_str_t* data);

#ifdef __cplusplus
}
#endif

#endif
