#include "user/user_get_prof.h"
#include "user/UserGetProfProcessor.h"
#include "app_common/app_log.h"
#include "app_common/common_func.h"
#include "proto/account.pb.h"

ngx_int_t set_get_user_prof_mysql_result(ngx_http_request_t* r, ngx_str_t* data)
{
    session_t* s = get_session_ptr(r);
    UserGetProfProcessor* processor = (UserGetProfProcessor*)s->processor;
    processor->handerSqlQueryResult(data);
    return NGX_OK;
}

ngx_int_t get_get_user_prof_resp(ngx_http_request_t* r, ngx_str_t* data)
{
    session_t* s = get_session_ptr(r);
    UserGetProfProcessor* processor = (UserGetProfProcessor*)s->processor;
    processor->buildResponse();
    ::Cell::GetUserProfileRespMsg* resp = (::Cell::GetUserProfileRespMsg*)processor->getResponse();
    std::string str_resp;
    resp->SerializeToString(&str_resp);
    data->len = str_resp.length();
    data->data = (u_char*)ngx_palloc(r->pool, data->len);
    ngx_memcpy(data->data, str_resp.c_str(), data->len);
 
    return NGX_OK;
}
