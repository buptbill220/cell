#include "pv_http_head.h"
#include "app_log.h"
#include <ngx_string.h>

static ngx_int_t ngx_http_header(ngx_list_part_t *part, u_char *name, size_t len, ngx_str_t *value)
{
    ngx_uint_t        i;
    ngx_table_elt_t  *header;
    header = part->elts;
    for (i = 0; /* void */; i++) {
        if (i >= part->nelts) {
            if (part->next == NULL) {
                break;   
            }
            part = part->next;
            header = part->elts;
            i = 0;   
        }
        if (header[i].hash == 0) {
            continue;   
        }
        if (len == header[i].key.len
                && ngx_strncasecmp(header[i].key.data, name, len) == 0)       
        {
            *value = header[i].value;
            return NGX_OK;       
        }    
    }    
    return NGX_DECLINED;
}

ngx_int_t ngx_http_header_in(ngx_http_request_t *r, u_char *name, size_t len, ngx_str_t *value)
{
        return ngx_http_header(&r->headers_in.headers.part, name, len, value);
}

ngx_int_t get_ip(ngx_http_request_t* r, ngx_str_t* ip)
{
    if (NGX_OK != ngx_http_header_in(r, (u_char*)"NS-Client-IP", sizeof("NS-Client-IP") - 1, ip)) {
        ngx_str_null(ip);
    }
    
    if (0 == ip->len) {
        *ip = r->connection->addr_text;
    }
    if (0 == ip->len) {
        LOG_ERROR(r->connection->log, "impossible: ip is null");
        return NGX_ERROR;
    }
#define MAX_IP_LEN (sizeof("255.255.255.255") - 1)
    if (ip->len > MAX_IP_LEN) {
        LOG_ERROR(r->connection->log, "ip len is too long<%V>", ip);
        return NGX_ERROR;
    }
    LOG_DEBUG(r->connection->log, "ip = %V", ip);
    return NGX_OK;
}
