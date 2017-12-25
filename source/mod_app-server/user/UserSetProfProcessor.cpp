#include "user/UserSetProfProcessor.h"
#include "app_common/app_log.h"
#include "app_common/common_func.h"
#include "app_common/ObjectPallocator.h"
#include "common/rapidjson/document.h"
#include <string>
#include "proto/common_data.pb.h"
#include "proto/account.pb.h"
#include <time.h>

UserSetProfProcessor::UserSetProfProcessor()
{
}

UserSetProfProcessor::~UserSetProfProcessor()
{
}

bool UserSetProfProcessor::init(ngx_http_request_t* r, session_t* s)
{
    ::Cell::SetUserProfileReqMsg* req = ObjectPallocator< ::Cell::SetUserProfileReqMsg>::allocate(r->pool);
    m_Response = (void*)ObjectPallocator< ::Cell::SetUserProfileRespMsg>::allocate(r->pool);
    if (!req->ParseFromArray(s->message_data.data, s->message_data.len)) {
        LOG_ERROR(r->connection->log, "parse SetUserProfileReqMsg failed");
        setRetcode(RET_INVALID_REQ);
        return false;
    }
    m_Request = (const void*)req;
    // set check user session query
    ngx_str_t redis_query_string;
    redis_query_string.data = (u_char*)ngx_palloc(r->pool, 128);
    u_char* p = ngx_snprintf(redis_query_string.data, 128, "get %d", req->uid());
    redis_query_string.len = p - redis_query_string.data;
    set_redis_query_string(r, redis_query_string);

    // set set user profile query string
    struct tm* tmp = localtime((const time_t*)&s->timestamp);
    u_char mod_time[32] = {0};
    u_char *pbuf = ngx_snprintf(mod_time, 32, "%04D-%02D-%02D %02D:%02D:%02D", tmp->tm_year + 1900, tmp->tm_mon + 1, tmp->tm_mday, tmp->tm_hour, tmp->tm_min, tmp->tm_sec);
    ngx_str_t mysql_query_string;
    mysql_query_string.data = (u_char*)ngx_palloc(r->pool, 512);
    p = ngx_snprintf(mysql_query_string.data, 512, "update user set avada_url = '%s', avada_size = '%s', user_name = '%s', "
            "mod_time = '%s' where uid = %d", req->avada_url().c_str(), req->avada_size().c_str(), req->usr_name().c_str(), 
            mod_time, req->uid());
    mysql_query_string.len = p - mysql_query_string.data;
    set_mysql_query_string(r, mysql_query_string);
    LOG_DEBUG(r->connection->log, "set mysql query string: %V", &mysql_query_string);

    return BaseProcessor::init(r, s);
}

void UserSetProfProcessor::clear()
{
    if (m_Request) {
        ::Cell::SetUserProfileReqMsg* req = (::Cell::SetUserProfileReqMsg*)m_Request;
        ObjectPallocator< ::Cell::SetUserProfileReqMsg>::free(req);
    }
    if (m_Response) {
        ::Cell::SetUserProfileRespMsg* resp = (::Cell::SetUserProfileRespMsg*)m_Response;
        ObjectPallocator< ::Cell::SetUserProfileRespMsg>::free(resp);
    }
}

void UserSetProfProcessor::handerSqlUpdateResult(ngx_str_t* result)
{
    if (result->data && result->len) {
        if (result->len == sizeof(ERROR_STRING) - 1 && 0 == ngx_strncmp(result->data, ERROR_STRING, result->len)) {
            LOG_ERROR(m_R->connection->log, "update mysql failed");
            setRetcode(RET_MYSQL_RESULT_INVALID);
            return ;
        }

        std::string json;
        json.assign((char*)result->data, result->len);
        rapidjson::Document dom;
        dom.Parse(json.c_str());
        
        if (dom.HasParseError()) {
            LOG_ERROR(m_R->connection->log, "parse json failed: %V", result);
            setRetcode(RET_MYSQL_RESULT_INVALID);
            return ;
        }

        if (!dom.IsObject() || dom["errcode"].GetUint() != 0) {
            LOG_ERROR(m_R->connection->log, "update false: %V", result);
            setRetcode(RET_MYSQL_RESULT_INVALID);
            return ;
        }
        
        if (!dom.HasMember("affected_rows")) {
            LOG_DEBUG(m_R->connection->log, "update nothing: %V", result);
            setRetcode(RET_MYSQL_RESULT_INVALID);
            return ;
        }

        return ;
    }
    setRetcode(RET_MYSQL_RESULT_INVALID);
}

void UserSetProfProcessor::buildResponse()
{
    ::Cell::SetUserProfileRespMsg* resp = (::Cell::SetUserProfileRespMsg*)m_Response;
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

ngx_str_t UserSetProfProcessor::getSessionToken() const
{
    ngx_str_t st = ngx_null_string;
    ::Cell::SetUserProfileReqMsg* req = (::Cell::SetUserProfileReqMsg*)m_Request;
    st.data = (u_char*)req->session_token().c_str();
    st.len = req->session_token().length();
    return st;
}
