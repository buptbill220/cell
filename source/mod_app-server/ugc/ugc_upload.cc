#include "ugc/ugc_upload.h"
#include "ugc/UgcUploadProcessor.h"
#include "app_common/common_func.h"
#include "proto/ugc.pb.h"
#include <string>

ngx_int_t set_upload_mysql_result(ngx_http_request_t* r, ngx_str_t* data)
{
    session_t* s = get_session_ptr(r);
    UgcUploadProcessor* processor = (UgcUploadProcessor*)s->processor;
    processor->handerInsertUgcResult(data);
    return NGX_OK;
}

ngx_int_t get_upload_mysql_result_status(ngx_http_request_t* r, ngx_str_t* data)
{
    session_t* s = get_session_ptr(r);
    UgcUploadProcessor* processor = (UgcUploadProcessor*)s->processor;
    if (RET_SUCCESS == processor->getRetcode()) {
        data->data = (u_char*)OK_STRING;
        data->len = sizeof(OK_STRING) - 1;
        return NGX_OK;
    }
    return NGX_ERROR;
}


ngx_int_t set_insert_message_achieve_result(ngx_http_request_t* r, ngx_str_t* data)
{
    session_t* s = get_session_ptr(r);
    UgcUploadProcessor* processor = (UgcUploadProcessor*)s->processor;
    processor->handerInsertAchieveResult(data);
    return NGX_OK;
}

ngx_int_t get_insert_message_achieve_result_status(ngx_http_request_t* r, ngx_str_t* data)
{
    session_t* s = get_session_ptr(r);
    UgcUploadProcessor* processor = (UgcUploadProcessor*)s->processor;
    if (RET_SUCCESS == processor->getRetcode()) {
        data->data = (u_char*)OK_STRING;
        data->len = sizeof(OK_STRING) - 1;
        return NGX_OK;
    }
    return NGX_ERROR;
}

ngx_int_t set_insert_message_seed_result(ngx_http_request_t* r, ngx_str_t* data)
{
    session_t* s = get_session_ptr(r);
    UgcUploadProcessor* processor = (UgcUploadProcessor*)s->processor;
    processor->handerInsertSeedResult(data);
    return NGX_OK;
}

ngx_int_t get_insert_message_seed_result_status(ngx_http_request_t* r, ngx_str_t* data)
{
    session_t* s = get_session_ptr(r);
    UgcUploadProcessor* processor = (UgcUploadProcessor*)s->processor;
    if (RET_SUCCESS == processor->getRetcode()) {
        data->data = (u_char*)OK_STRING;
        data->len = sizeof(OK_STRING) - 1;
        return NGX_OK;
    }
    return NGX_ERROR;
}

ngx_int_t set_upload_get_user_achieve_result(ngx_http_request_t* r, ngx_str_t* data)
{
    session_t* s = get_session_ptr(r);
    UgcUploadProcessor* processor = (UgcUploadProcessor*)s->processor;
    processor->handerUserAchieveResult(data);
    return NGX_OK;
}

ngx_int_t get_upload_get_user_achieve_result_status(ngx_http_request_t* r, ngx_str_t* data)
{
    session_t* s = get_session_ptr(r);
    UgcUploadProcessor* processor = (UgcUploadProcessor*)s->processor;
    u_char* redis_status = (u_char*)processor->getData("redis_status");
    if (redis_status) {
        data->data = (u_char*)OK_STRING;
        data->len = sizeof(OK_STRING) - 1;
        return NGX_OK;
    }
    return NGX_ERROR;
}

ngx_int_t get_upload_resp(ngx_http_request_t* r, ngx_str_t* data)
{
    session_t* s = get_session_ptr(r);
    UgcUploadProcessor* processor = (UgcUploadProcessor*)s->processor;
    processor->buildResponse();
    ::Cell::UploadUGCRespMsg* resp = (::Cell::UploadUGCRespMsg*)processor->getResponse();
    std::string str_resp;
    resp->SerializeToString(&str_resp);
    data->len = str_resp.length();
    data->data = (u_char*)ngx_palloc(r->pool, data->len);
    ngx_memcpy(data->data, str_resp.c_str(), data->len);

    return NGX_OK;
}
