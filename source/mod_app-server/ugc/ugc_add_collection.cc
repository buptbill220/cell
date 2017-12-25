#include "ugc/ugc_add_collection.h"
#include "ugc/UgcAddCollectionProcessor.h"
#include "app_common/common_func.h"
#include "proto/ugc.pb.h"
#include <string>

ngx_int_t set_query_clt_mysql_result(ngx_http_request_t* r, ngx_str_t* data)
{
    session_t* s = get_session_ptr(r);
    UgcAddCollectionProcessor* processor = (UgcAddCollectionProcessor*)s->processor;
    processor->handleQueryCltResult(data);
    return NGX_OK;
}

ngx_int_t get_add_collection_status(ngx_http_request_t* r, ngx_str_t* data)
{
    session_t* s = get_session_ptr(r);
    UgcAddCollectionProcessor* processor = (UgcAddCollectionProcessor*)s->processor;
    u_char* status = (u_char*)processor->getData("status");
    if (status) {
        data->data = (u_char*)OK_STRING;
        data->len = sizeof(OK_STRING) - 1;
    } else {
        data->data = (u_char*)ERROR_STRING;
        data->len = sizeof(ERROR_STRING) - 1;
    }
    return NGX_OK;
}

ngx_int_t set_add_clt_mysql_result(ngx_http_request_t* r, ngx_str_t* data)
{
    session_t* s = get_session_ptr(r);
    UgcAddCollectionProcessor* processor = (UgcAddCollectionProcessor*)s->processor;
    processor->handleAddCltResult(data);
    return NGX_OK;
}

ngx_int_t get_add_collection_resp(ngx_http_request_t* r, ngx_str_t* data)
{
    session_t* s = get_session_ptr(r);
    UgcAddCollectionProcessor* processor = (UgcAddCollectionProcessor*)s->processor;
    processor->buildResponse();
    ::Cell::AddCollectionRespMsg* resp = (::Cell::AddCollectionRespMsg*)processor->getResponse();
    std::string str_resp;
    resp->SerializeToString(&str_resp);
    data->len = str_resp.length();
    data->data = (u_char*)ngx_palloc(r->pool, data->len);
    ngx_memcpy(data->data, str_resp.c_str(), data->len);

    return NGX_OK;
}
