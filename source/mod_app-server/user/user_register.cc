#include "user/user_register.h"
#include "app_common/common_func.h"
#include "user/UserRegisterProcessor.h"
#include "proto/initial.pb.h"

ngx_int_t set_register_mysql_result(ngx_http_request_t* r, ngx_str_t* data)
{
    session_t* s = get_session_ptr(r);
    UserRegisterProcessor* processor = (UserRegisterProcessor*)s->processor;
    processor->handerSqlInsertResult(data);
    return NGX_OK;
}

ngx_int_t get_register_resp(ngx_http_request_t* r, ngx_str_t* data)
{
    session_t* s = get_session_ptr(r);
    UserRegisterProcessor* processor = (UserRegisterProcessor*)s->processor;
    processor->buildResponse();
    ::Cell::RegRespMsg* resp = (::Cell::RegRespMsg*)processor->getResponse();
    std::string str_resp;
    resp->SerializeToString(&str_resp);
    data->len = str_resp.length();
    data->data = (u_char*)ngx_palloc(r->pool, data->len);
    ngx_memcpy(data->data, str_resp.c_str(), data->len);
    return NGX_OK;
}
