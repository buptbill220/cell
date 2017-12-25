#include "user/UserCheckRegProcessor.h"
#include "app_common/ObjectPallocator.h"
#include "app_common/app_log.h"
#include "app_common/common_func.h"
#include "proto/common_data.pb.h"
#include "proto/initial.pb.h"
#include "common/rapidjson/document.h"
#include <string>

UserCheckRegProcessor::UserCheckRegProcessor()
{
}

UserCheckRegProcessor::~UserCheckRegProcessor()
{
}

bool UserCheckRegProcessor::init(ngx_http_request_t* r, session_t* s)
{
    ::Cell::CheckUserRegReqMsg* req = ObjectPallocator< ::Cell::CheckUserRegReqMsg>::allocate(r->pool);
    m_Response = (void*)ObjectPallocator< ::Cell::CheckUserRegRespMsg>::allocate(r->pool);
    if (!req->ParseFromArray(s->message_data.data, s->message_data.len)) {
        LOG_ERROR(r->connection->log, "parse CheckUserRegReqMsg failed");
        setRetcode(RET_INVALID_REQ);
        return false;
    }

    m_Request = (const void*)req;
    ngx_str_t mysql_query_string;
    mysql_query_string.data = (u_char*)ngx_palloc(r->pool, 256);
    u_char* p = ngx_snprintf(mysql_query_string.data, 256, "select 1 from user where phone = '%s' limit 1", req->usr_info().c_str());
    mysql_query_string.len = p - mysql_query_string.data;
    set_mysql_query_string(r, mysql_query_string);
    
    return BaseProcessor::init(r, s);
}

void UserCheckRegProcessor::clear()
{
    if (m_Request) {
        ::Cell::CheckUserRegReqMsg* req = (::Cell::CheckUserRegReqMsg*)m_Request;
        ObjectPallocator< ::Cell::CheckUserRegReqMsg>::free(req);
    }
    if (m_Response) {
        ::Cell::CheckUserRegRespMsg* resp = (::Cell::CheckUserRegRespMsg*)m_Response;
        ObjectPallocator< ::Cell::CheckUserRegRespMsg>::free(resp);
    }
}

void UserCheckRegProcessor::handerSqlQueryResult(ngx_str_t* result)
{
    if (result->data && result->len) {
        if (result->len == sizeof(ERROR_STRING) - 1 && 
                0 == ngx_strncmp(result->data, ERROR_STRING, result->len)) {
            LOG_ERROR(m_R->connection->log, "mysql query error");
            setRetcode(RET_MYSQL_RESULT_INVALID);
            return ;
        }

        rapidjson::Document dom;
        std::string json;
        json.assign((char*)result->data, result->len);
        dom.Parse(json.c_str());

        if (dom.HasParseError() || !dom.IsArray()) {
            LOG_ERROR(m_R->connection->log, "json parse error, json: %V", result);
            setRetcode(RET_MYSQL_RESULT_INVALID);
            return ;
        }

        ::Cell::CheckUserRegRespMsg* resp = (::Cell::CheckUserRegRespMsg*)m_Response;
        if (0 == dom.Size()) {
            resp->set_user_stat(::Cell::US_UNREG_USER);
            return ;
        }

        resp->set_user_stat(::Cell::US_REG_USER);
        return ;
    }
    setRetcode(RET_MYSQL_RESULT_INVALID);
}

void UserCheckRegProcessor::buildResponse()
{
    ::Cell::CheckUserRegRespMsg* resp = (::Cell::CheckUserRegRespMsg*)m_Response;
    switch (m_Retcode) {
        case RET_SUCCESS:
            resp->set_ret_code(::Cell::RC_SUCCESS);
            break;
        case RET_SESSION_INVALID:
            resp->set_ret_code(::Cell::RC_SESSION_INVALID);
            break;
        default:
            resp->set_ret_code(::Cell::RC_FAILED);
    }
}
