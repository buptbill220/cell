#include "user/UserRegisterProcessor.h"
#include "proto/common_data.pb.h"
#include "proto/initial.pb.h"
#include "app_common/ObjectPallocator.h"
#include "app_common/app_log.h"
#include "app_common/common_func.h"
#include "common/rapidjson/document.h"
#include <string>
#include <time.h>

#define DEFAULT_IMG "default/default.jpg"

UserRegisterProcessor::UserRegisterProcessor()
{
}

UserRegisterProcessor::~UserRegisterProcessor()
{
}

bool UserRegisterProcessor::init(ngx_http_request_t* r, session_t* s)
{
    ::Cell::RegReqMsg* req = ObjectPallocator< ::Cell::RegReqMsg>::allocate(r->pool);
    m_Response = (void*)ObjectPallocator< ::Cell::RegRespMsg>::allocate(r->pool);
    if (!req) {
        LOG_ERROR(r->connection->log, "palloc RegReqMsg failed");
        setRetcode(RET_INTERNAL_ERROR);
        return false;
    }
    if (!req->ParseFromArray(s->message_data.data, s->message_data.len)) {
        LOG_ERROR(r->connection->log, "parse RegReqMsg failed");
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

    // set register user insert sql query
    struct tm *tmp = localtime((const time_t*)&s->timestamp);
    u_char add_time[32] = {0};
    u_char *pbuf = ngx_snprintf(add_time, 32, "%04D-%02D-%02D %02D:%02D:%02D", tmp->tm_year + 1900, tmp->tm_mon + 1, tmp->tm_mday, tmp->tm_hour, tmp->tm_min, tmp->tm_sec);
    const char* img = req->avada_url().empty() ? DEFAULT_IMG : req->avada_url().c_str();
    ngx_str_t mysql_query_string;
    mysql_query_string.data = (u_char*)ngx_palloc(r->pool, 512);
    p = ngx_snprintf(mysql_query_string.data, 1024, "update user set passwd = '%s', phone = '%s', email = '%s', gender = %d, "
            "birthday = '%s', avada_url = '%s', avada_size = '%s', user_name = '%s', reg_source = %d, blood_type = %d, "
            "constellation = %d, zodiac = %d, add_time = '%s', mod_time = '%s', latitude = %.6f, longitude = %.6f, "
            "status = 1 where uid = %D and phone is NULL", req->password().c_str(), req->phone().c_str(), req->email().c_str(), 
            req->gender(), req->birthday().c_str(), img, req->avada_size().c_str(), req->usr_name().c_str(), 
            req->reg_source(), req->blood_type(), req->constellation(), req->zodiac(), add_time, add_time, req->geo().latitude(), 
            req->geo().longitude(), req->uid());
    mysql_query_string.len = p - mysql_query_string.data;
    set_mysql_query_string(r, mysql_query_string);
    LOG_DEBUG(r->connection->log, "set mysql query string: %V", &mysql_query_string);

    return  BaseProcessor::init(r, s);
}

void UserRegisterProcessor::clear()
{
    if (m_Request) {
        ::Cell::RegReqMsg* req = (::Cell::RegReqMsg*)m_Request;
        ObjectPallocator< ::Cell::RegReqMsg>::free(req);
    }
    if (m_Response) {
        ::Cell::RegRespMsg* res = (::Cell::RegRespMsg*)m_Response;
        ObjectPallocator< ::Cell::RegRespMsg>::free(res);
    }
}

void UserRegisterProcessor::handerSqlInsertResult(ngx_str_t* result)
{
    if (result->data && result->len) {
        if (result->len == sizeof(ERROR_STRING) - 1 && 
                0 == ngx_strncmp(result->data, ERROR_STRING, result->len)) {
            LOG_ERROR(m_R->connection->log, "update user error");
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
            LOG_ERROR(m_R->connection->log, "parse json failed: %V", result);
            setRetcode(RET_MYSQL_RESULT_INVALID);
            return ;
        }

        if (!dom.HasMember("affected_rows")) {
            LOG_DEBUG(m_R->connection->log, "update nothing: %V", result);
            setRetcode(RET_MYSQL_RESULT_INVALID);
            return ;
        }
        LOG_DEBUG(m_R->connection->log, "insert user successfully");
        return ;
    }
    setRetcode(RET_MYSQL_RESULT_INVALID);
}

void UserRegisterProcessor::buildResponse()
{
    ::Cell::RegRespMsg* resp = (::Cell::RegRespMsg*)m_Response;
    switch (m_Retcode) {
        case RET_SUCCESS:
            resp->set_ret_code(::Cell::RC_SUCCESS);
            resp->set_user_stat(::Cell::US_REG_USER);
            break;
        case RET_SESSION_INVALID:
            resp->set_ret_code(::Cell::RC_SESSION_INVALID);
            break;
        default:
            resp->set_ret_code(::Cell::RC_FAILED);
    }
}

ngx_str_t UserRegisterProcessor::getSessionToken() const
{
    ngx_str_t st = ngx_null_string;
    ::Cell::RegReqMsg* req = (::Cell::RegReqMsg*)m_Request;
    st.data = (u_char*)req->session_token().c_str();
    st.len = req->session_token().length();
    return st;
}

ngx_uint_t UserRegisterProcessor::getUid()
{
    ::Cell::RegReqMsg* req = (::Cell::RegReqMsg*)m_Request;
    return req->uid();
}
