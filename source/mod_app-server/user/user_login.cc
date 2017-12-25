#include "user/user_login.h"
#include "app_common/common_func.h"
#include "app_common/BaseProcessor.h"
#include "user/UserLoginProcessor.h"
#include "proto/common_data.pb.h"
#include "proto/initial.pb.h"

ngx_int_t set_login_mysql_query_result(ngx_http_request_t* r, ngx_str_t* data)
{
    session_t* s = get_session_ptr(r);
    UserLoginProcessor* processor = (UserLoginProcessor*)s->processor;
    processor->handerSqlQueryResult(data);
    return NGX_OK;
}

ngx_int_t get_login_mysql_query_status(ngx_http_request_t* r, ngx_str_t* data)
{
    session_t* s = get_session_ptr(r);
    UserLoginProcessor* processor = (UserLoginProcessor*)s->processor;
    if (RET_SUCCESS == processor->getRetcode()) {
        data->data = (u_char*)OK_STRING;
        data->len = sizeof(OK_STRING) - 1;
    } else {
        data->data = (u_char*)ERROR_STRING;
        data->len = sizeof(ERROR_STRING) - 1;
    }
    return NGX_OK;
}

ngx_int_t get_login_resp(ngx_http_request_t* r, ngx_str_t* data)
{
    session_t* s = get_session_ptr(r);
    UserLoginProcessor* processor = (UserLoginProcessor*)s->processor;
    processor->buildResponse();
    ::Cell::LoginRespMsg* resp = (::Cell::LoginRespMsg*)processor->getResponse();
    std::string str_resp;
    resp->SerializeToString(&str_resp);
    data->len = str_resp.length();
    data->data = (u_char*)ngx_palloc(r->pool, data->len);
    ngx_memcpy(data->data, str_resp.c_str(), data->len);
    return NGX_OK;
}
