#include "user/user_get_ugc_list.h"
#include "user/UserGetUgcListProcessor.h"
#include "app_common/common_func.h"
#include "app_common/app_log.h"
#include "proto/ugc.pb.h"

ngx_int_t set_get_user_ugc_list_result(ngx_http_request_t* r, ngx_str_t* data)
{
    session_t* s = get_session_ptr(r);
    UserGetUgcListProcessor* processor = (UserGetUgcListProcessor*)s->processor;
    processor->handerUgcListResult(data);
        return NGX_OK;
}

ngx_int_t get_get_user_ugc_list_resp(ngx_http_request_t* r, ngx_str_t* data)
{
    session_t* s = get_session_ptr(r);
    UserGetUgcListProcessor* processor = (UserGetUgcListProcessor*)s->processor;
    processor->buildResponse();
    ::Cell::GetUGCListRespMsg* resp = (::Cell::GetUGCListRespMsg*)processor->getResponse();
    std::string str_resp;
    resp->SerializeToString(&str_resp);
    data->len = str_resp.length();
    data->data = (u_char*)ngx_palloc(r->pool, data->len);
    ngx_memcpy(data->data, str_resp.c_str(), data->len);
 
    return NGX_OK;
}
