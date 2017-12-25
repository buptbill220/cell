#include "user/user_init.h"
#include "user/UserInitProcessor.h"
#include "app_common/common_func.h"
#include "proto/initial.pb.h"

ngx_int_t set_init_mysql_query_result(ngx_http_request_t* r, ngx_str_t* data)
{
    session_t* s = get_session_ptr(r);
    UserInitProcessor* processor = (UserInitProcessor*)s->processor;
    processor->handerSqlQueryResult(data);
    return NGX_OK;
}

ngx_int_t get_init_mysql_query_status(ngx_http_request_t* r, ngx_str_t* data)
{
    session_t* s = get_session_ptr(r);
    UserInitProcessor* processor = (UserInitProcessor*)s->processor;
    if (RET_SUCCESS != processor->getRetcode()) {
        data->data = (u_char*)ERROR_STRING;
        data->len = sizeof(ERROR_STRING) - 1;
    } else {
        ngx_uint_t uid = processor->getUid();
        if (0 == uid) {
            data->data = (u_char*)EMPTY_STRING;
            data->len = sizeof(EMPTY_STRING) - 1;
            return NGX_OK;
        }
        data->data = (u_char*)OK_STRING;
        data->len = sizeof(OK_STRING) - 1;
    }
    return NGX_OK;
}

ngx_int_t set_init_mysql_insert_result(ngx_http_request_t* r, ngx_str_t* data)
{
    session_t* s = get_session_ptr(r);
    UserInitProcessor* processor = (UserInitProcessor*)s->processor;
    processor->handerSqlInsertResult(data);
    return NGX_OK;
}

ngx_int_t get_init_resp(ngx_http_request_t* r, ngx_str_t* data)
{
    session_t* s = get_session_ptr(r);
    UserInitProcessor* processor = (UserInitProcessor*)s->processor;
    processor->buildResponse();
    ::Cell::InitReqMsg* req = (::Cell::InitReqMsg*)processor->getResponse();
    std::string str_resp;
    req->SerializeToString(&str_resp);
    data->len = str_resp.length();
    data->data = (u_char*)ngx_palloc(r->pool, data->len);
    ngx_memcpy(data->data, (u_char*)str_resp.c_str(), data->len);
    return NGX_OK;
}
