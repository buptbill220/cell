#include "ugc/ugc_get_comment.h"
#include "ugc/UgcGetCommentProcessor.h"
#include "app_common/common_func.h"
#include "proto/ugc.pb.h"
#include <string>

ngx_int_t set_get_comment_mysql_result(ngx_http_request_t* r, ngx_str_t* data)
{
    session_t* s = get_session_ptr(r);
    UgcGetCommentProcessor* processor = (UgcGetCommentProcessor*)s->processor;
    processor->handerMysqlResult(data);
    return NGX_OK;
}

ngx_int_t get_get_comment_resp(ngx_http_request_t* r, ngx_str_t* data)
{
    session_t* s = get_session_ptr(r);
    UgcGetCommentProcessor* processor = (UgcGetCommentProcessor*)s->processor;
    processor->buildResponse();
    ::Cell::GetCommentRespMsg* resp = (::Cell::GetCommentRespMsg*)processor->getResponse();
    std::string str_resp;
    resp->SerializeToString(&str_resp);
    data->len = str_resp.length();
    data->data = (u_char*)ngx_palloc(r->pool, data->len);
    ngx_memcpy(data->data, str_resp.c_str(), data->len);

    return NGX_OK;
}
