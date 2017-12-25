#include "ugc/ugc_post.h"
#include "ugc/UgcPostProcessor.h"
#include "app_common/common_func.h"
#include "app_common/app_log.h"
#include "common/rapidjson/document.h"
#include "proto/ugc.pb.h"
#include <string>

ngx_int_t set_post_ugc_mysql_result(ngx_http_request_t* r, ngx_str_t* data)
{
    session_t* s = get_session_ptr(r);
    UgcPostProcessor* processor = (UgcPostProcessor*)s->processor;
    processor->handerInsertPostResult(data);
    return NGX_OK;
}

ngx_int_t get_post_ugc_mysql_result_status(ngx_http_request_t* r, ngx_str_t* data)
{
    session_t* s = get_session_ptr(r);
    UgcPostProcessor* processor = (UgcPostProcessor*)s->processor;
    if (RET_SUCCESS == processor->getRetcode()) {
        data->data = (u_char*)OK_STRING;
        data->len = sizeof(OK_STRING) - 1;
        return NGX_OK;
    }
    return NGX_ERROR;
}

ngx_int_t get_post_ugc_type(ngx_http_request_t* r, ngx_str_t* data)
{
    session_t* s = get_session_ptr(r);
    UgcPostProcessor* processor = (UgcPostProcessor*)s->processor;
    ::Cell::PostUGCReqMsg* req = (::Cell::PostUGCReqMsg*)processor->getRequest();
    if (::Cell::PT_POST == req->post_type()) {
        data->data = (u_char*)"post";
        data->len = sizeof("post") - 1;
        processor->handerPostAction();
    } else {
        data->data = (u_char*)"pass";
        data->len = sizeof("pass") - 1;
        processor->handerPassAction();
    }
    return NGX_OK;
}

ngx_int_t set_post_ugc_select_achieve_type(ngx_http_request_t* r, ngx_str_t* data)
{
    LOG_DEBUG(r->connection->log, "select achieve type: %V", data);
    session_t* s = get_session_ptr(r);
    UgcPostProcessor* processor = (UgcPostProcessor*)s->processor;
    processor->handerUgcSelectAchieveType(data);
    return NGX_OK;
}

ngx_int_t set_post_ugc_select_achieve_result(ngx_http_request_t* r, ngx_str_t* data)
{
    session_t* s = get_session_ptr(r);
    UgcPostProcessor* processor = (UgcPostProcessor*)s->processor;
    processor->handerUgcSelectAcieveResult(data);
    return NGX_OK;
}

ngx_int_t get_post_ugc_select_achieve_result_status(ngx_http_request_t* r, ngx_str_t* data)
{
    session_t* s = get_session_ptr(r);
    UgcPostProcessor* processor = (UgcPostProcessor*)s->processor;
    if (RET_SUCCESS == processor->getRetcode()) {
        data->data = (u_char*)OK_STRING;
        data->len = sizeof(OK_STRING) - 1;
        return NGX_OK;
    }
    return NGX_ERROR;
}

ngx_int_t set_post_ugc_update_achieve_type(ngx_http_request_t* r, ngx_str_t* data)
{
    session_t* s = get_session_ptr(r);
    UgcPostProcessor* processor = (UgcPostProcessor*)s->processor;
    processor->handerUgcUpdateAchieveType(data);
    return NGX_OK;
}

ngx_int_t set_post_ugc_update_achieve_result(ngx_http_request_t* r, ngx_str_t* data)
{
    session_t* s = get_session_ptr(r);
    UgcPostProcessor* processor = (UgcPostProcessor*)s->processor;
    processor->handerUgcUpdateAchieveResult(data);
    return NGX_OK;
}

ngx_int_t get_post_ugc_seed_is_local(ngx_http_request_t* r, ngx_str_t* data)
{
    session_t* s = get_session_ptr(r);
    UgcPostProcessor* processor = (UgcPostProcessor*)s->processor;
    processor->handerLocalSeed();
    u_char* local_seed = (u_char*)processor->getData("local_seed");
    if (local_seed) {
        data->data = (u_char*)OK_STRING;
        data->len = sizeof(OK_STRING) - 1;
    } else {
        data->data = (u_char*)ERROR_STRING;
        data->len = sizeof(ERROR_STRING) - 1;
    }
    return NGX_OK;
}

ngx_int_t set_post_ugc_user_achieve_type(ngx_http_request_t* r, ngx_str_t* data)
{
    session_t* s = get_session_ptr(r);
    UgcPostProcessor* processor = (UgcPostProcessor*)s->processor;
    processor->handerUserAchieveType(data);
    return NGX_OK;
}


ngx_int_t set_post_ugc_user_achieve_result(ngx_http_request_t* r, ngx_str_t* data)
{
    session_t* s = get_session_ptr(r);
    UgcPostProcessor* processor = (UgcPostProcessor*)s->processor;
    processor->handerUserAchieveResult(data);
    return NGX_OK;
}

ngx_int_t get_post_ugc_user_achieve_result_status(ngx_http_request_t* r, ngx_str_t* data)
{
    session_t* s = get_session_ptr(r);
    UgcPostProcessor* processor = (UgcPostProcessor*)s->processor;
    if (RET_SUCCESS == processor->getRetcode()) {
        data->data = (u_char*)OK_STRING;
        data->len = sizeof(OK_STRING) - 1;
    }
    return NGX_OK;
}

ngx_int_t get_post_ugc_resp(ngx_http_request_t* r, ngx_str_t* data)
{
    session_t* s = get_session_ptr(r);
    UgcPostProcessor* processor = (UgcPostProcessor*)s->processor;
    processor->buildResponse();
    ::Cell::PostUGCRespMsg* resp = (::Cell::PostUGCRespMsg*)processor->getResponse();
    std::string str_resp;
    resp->SerializeToString(&str_resp);
    data->len = str_resp.length();
    data->data = (u_char*)ngx_palloc(r->pool, data->len);
    ngx_memcpy(data->data, str_resp.c_str(), data->len);
    
    return NGX_OK;
}
