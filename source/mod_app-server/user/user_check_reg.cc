#include "user/user_check_reg.h"
#include "user/UserCheckRegProcessor.h"
#include "app_common/common_func.h"
#include "proto/initial.pb.h"
#include <string>

ngx_int_t set_check_user_reg_result(ngx_http_request_t* r, ngx_str_t* data)
{
    session_t* s = get_session_ptr(r);
    UserCheckRegProcessor* processor = (UserCheckRegProcessor*)s->processor;
    processor->handerSqlQueryResult(data);
    return NGX_OK;
}

ngx_int_t get_check_user_reg_resp(ngx_http_request_t* r, ngx_str_t* data)
{
    session_t* s = get_session_ptr(r);
    UserCheckRegProcessor* processor = (UserCheckRegProcessor*)s->processor;
    processor->buildResponse();
    ::Cell::CheckUserRegRespMsg* req = (::Cell::CheckUserRegRespMsg*)processor->getResponse();
    std::string str_resp;
    req->SerializeToString(&str_resp);
    data->len = str_resp.length();
    data->data = (u_char*)ngx_palloc(r->pool, data->len);
    ngx_memcpy(data->data, (u_char*)str_resp.c_str(), data->len);
    
    return NGX_OK;
}
