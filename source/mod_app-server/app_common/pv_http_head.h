/**
 *  Copyright (c) 2015 cell-app.com
 *  All rights reserved.
 *
 *  author: fangming
 *  date: 2015.02.05
 *  
 */

#ifndef _PV_HTTP_HEAD_H_
#define _PV_HTTP_HEAD_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "global.h"

ngx_int_t ngx_http_header_in(ngx_http_request_t *r, u_char *name, size_t len, ngx_str_t *value);
ngx_int_t get_ip(ngx_http_request_t* r, ngx_str_t* t);

#ifdef __cplusplus
}
#endif

#endif
