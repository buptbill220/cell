#include "reg_var.h"

void 
var_set(ngx_http_request_t* r,
        ngx_http_variable_value_t* v, 
        uintptr_t data)
{
    variable_map_t* vp;
    ngx_str_t value;

    vp = (variable_map_t*)data;
    if (NULL == vp) {
        ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "impossible: var is null");
        return ;
    }
    value.data = v->data;
    value.len  = v->len;

    if (vp->set && NGX_OK != vp->set(r, &value)) {
        return ;
    }
    ngx_log_error(NGX_LOG_DEBUG, r->connection->log, 0,
            "set var<%V=%p> ok, in %V", &vp->name, v->data, &r->uri);
    return ;
}

static ngx_int_t 
var_get(ngx_http_request_t* r, 
        ngx_http_variable_value_t* v, 
        uintptr_t data)
{
    variable_map_t* vp;
    ngx_str_t value = ngx_null_string;

    vp = (variable_map_t*)data;
    if (NULL == vp) {
        ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "impossible: vp is null");
        goto NOT_FOUND;
    }   
    if (vp->get && NGX_OK != vp->get(r, &value)) {
        goto NOT_FOUND;
    }
    v->data = value.data;
    v->len  = value.len;
    v->valid = 1;
    v->no_cacheable = 0;
    v->not_found = 0;
    v->escape = 0;
    ngx_log_error(NGX_LOG_DEBUG, r->connection->log, 0,
            "get var<%V=%p> ok, in %V", &vp->name, v->data, &r->uri);
    return NGX_OK;

NOT_FOUND:
    v->not_found = 1;
    return NGX_ERROR;
}

void* ngx_http_register_vars(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_http_variable_t* var;
    variable_map_t* vp;

    for (vp = cmd->post; vp->name.len; ++vp) {
        ngx_uint_t flag = NGX_HTTP_VAR_INDEXED;
        if (vp->set) {
            flag |= NGX_HTTP_VAR_CHANGEABLE;
            flag |= NGX_HTTP_VAR_NOCACHEABLE;
        }
        var = ngx_http_add_variable(cf, &vp->name, flag);
        if (NULL == var) {
            return (char*)"add variable error";
        }
        var->get_handler = var_get;
        var->set_handler = var_set;
        var->data = (uintptr_t)vp;
        ngx_http_get_variable_index(cf, &vp->name);
    }
    return NGX_CONF_OK;
}
