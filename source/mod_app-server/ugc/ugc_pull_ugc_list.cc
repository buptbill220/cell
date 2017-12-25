#include "ugc/ugc_pull_ugc_list.h"
#include "ugc/UgcPullListProcessor.h"
#include "app_common/app_log.h"
#include "app_common/common_func.h"
#include "proto/ugc.pb.h"
#include <string>


ngx_int_t set_pull_ugc_list_type(ngx_http_request_t* r, ngx_str_t* data)
{
    session_t* s = get_session_ptr(r);
    UgcPullListProcessor* processor = (UgcPullListProcessor*)s->processor;
    processor->handerUgcType(data);
    return NGX_OK;
}

ngx_int_t set_pull_ugc_list_mysql_result(ngx_http_request_t* r, ngx_str_t* data)
{
    session_t* s = get_session_ptr(r);
    UgcPullListProcessor* processor = (UgcPullListProcessor*)s->processor;
    processor->handerUgcMysqlResult(data);
    return NGX_OK;
}

ngx_int_t get_pull_ugc_list_has_ugc(ngx_http_request_t* r, ngx_str_t* data)
{
    session_t* s = get_session_ptr(r);
    UgcPullListProcessor* processor = (UgcPullListProcessor*)s->processor;
    u_char* ugc_status = (u_char*)processor->getData("ugc_status");
    if (ugc_status && 0 == ngx_strncmp(ugc_status, OK_STRING, sizeof(OK_STRING) - 1)) {
        data->data = (u_char*)OK_STRING;
        data->len = sizeof(OK_STRING) - 1;
    } else {
        data->data = (u_char*)ERROR_STRING;
        data->len = sizeof(ERROR_STRING) - 1;
    }
    return NGX_OK;
}

ngx_int_t get_pull_ugc_pull_type(ngx_http_request_t* r, ngx_str_t* data)
{
    session_t* s = get_session_ptr(r);
    UgcPullListProcessor* processor = (UgcPullListProcessor*)s->processor;
    const ::Cell::PullUGCReqMsg* req = (const ::Cell::PullUGCReqMsg*)processor->getRequest();
    if (req->pull_type() == ::Cell::PT_PLAY) {
        data->data = (u_char*)"play";
        data->len = sizeof("play") - 1;
    } else {
        data->data = (u_char*)"pull";
        data->len = sizeof("pull") - 1;
    }
    return NGX_OK;
}

ngx_int_t get_pull_ugc_list_resp(ngx_http_request_t* r, ngx_str_t* data)
{
    session_t* s = get_session_ptr(r);
    UgcPullListProcessor* processor = (UgcPullListProcessor*)s->processor;
    processor->buildResponse();
    ::Cell::PullUGCRespMsg* resp = (::Cell::PullUGCRespMsg*)processor->getResponse();
    std::string str_resp;
    resp->SerializeToString(&str_resp);
    data->len = str_resp.length();
    data->data = (u_char*)ngx_palloc(r->pool, data->len);
    ngx_memcpy(data->data, str_resp.c_str(), data->len);
    return NGX_OK;
}
