#include "user/UserGetProfProcessor.h"
#include "app_common/app_log.h"
#include "app_common/common_func.h"
#include "app_common/ObjectPallocator.h"
#include "proto/common_data.pb.h"
#include "proto/account.pb.h"
#include "common/rapidjson/document.h"
#include <string.h>

UserGetProfProcessor::UserGetProfProcessor()
{
}

UserGetProfProcessor::~UserGetProfProcessor()
{
}

bool UserGetProfProcessor::init(ngx_http_request_t* r, session_t* s)
{
    ::Cell::GetUserProfileReqMsg* req = ObjectPallocator< ::Cell::GetUserProfileReqMsg>::allocate(r->pool);
    m_Response = (void*)ObjectPallocator< ::Cell::GetUserProfileRespMsg>::allocate(r->pool);
    if (!req->ParseFromArray(s->message_data.data, s->message_data.len)) {
        LOG_ERROR(r->connection->log, "parse GetUserProfileReqMsg failed");
        setRetcode(RET_INVALID_REQ);
        return false;
    }
    if (::Cell::US_PROFILE != req->usr_prf_type()) {
        LOG_ERROR(r->connection->log, "usr_prf_type is not US_PROFILE, but is: %D", req->usr_prf_type());
        setRetcode(RET_INVALID_REQ);
        return false;
    }
    m_Request = (const void*)req;
    // set check user session query
    ngx_str_t redis_query_string;
    redis_query_string.data = (u_char*)ngx_palloc(r->pool, 128);
    u_char* p = ngx_snprintf(redis_query_string.data, 128, "get %D", req->uid());
    redis_query_string.len = p - redis_query_string.data;
    set_redis_query_string(r, redis_query_string);

    // set set user profile query string
    ngx_str_t mysql_query_string;
    mysql_query_string.data = (u_char*)ngx_palloc(r->pool, 512);
    p = ngx_snprintf(mysql_query_string.data, 512, "select phone, email, gender, birthday, " \
            "avada_url, avada_size, user_name, blood_type, constellation, zodiac from user where uid = %D", req->org_uid());
    mysql_query_string.len = p - mysql_query_string.data;
    set_mysql_query_string(r, mysql_query_string);
    LOG_DEBUG(r->connection->log, "set mysql query string: %V", &mysql_query_string);

    return BaseProcessor::init(r, s);

}

void UserGetProfProcessor::clear()
{
    if (m_Request) {
        ::Cell::GetUserProfileReqMsg* req = (::Cell::GetUserProfileReqMsg*)m_Request;
        ObjectPallocator< ::Cell::GetUserProfileReqMsg>::free(req);
    }
    if (m_Response) {
        ::Cell::GetUserProfileRespMsg* resp = (::Cell::GetUserProfileRespMsg*)m_Response;
        ObjectPallocator< ::Cell::GetUserProfileRespMsg>::free(resp);
    }
}

ngx_str_t UserGetProfProcessor::getSessionToken() const
{
    ngx_str_t st = ngx_null_string;
    ::Cell::GetUserProfileReqMsg* req = (::Cell::GetUserProfileReqMsg*)m_Request;
    st.data = (u_char*)req->session_token().c_str();
    st.len = req->session_token().length();
    return st;
}

void UserGetProfProcessor::handerSqlQueryResult(ngx_str_t* result)
{
    if (result->data && result->len) {
        if (result->len == sizeof(ERROR_STRING) - 1 && 
                0 == ngx_strncmp(result->data, ERROR_STRING, result->len)) {
            LOG_ERROR(m_R->connection->log, "query mysql failed");
            setRetcode(RET_MYSQL_RESULT_INVALID);
            return ;
        }
        rapidjson::Document dom;
        std::string json;
        json.assign((char*)result->data, result->len);
        dom.Parse(json.c_str());
        if (dom.HasParseError() || !dom.IsArray()) {
            LOG_ERROR(m_R->connection->log, "parse mysql result json failed: %V", result);
            setRetcode(RET_MYSQL_RESULT_INVALID);
            return ;
        }
        if (0 == dom.Size() || !dom[0].HasMember("phone")) {
            LOG_ERROR(m_R->connection->log, "mysql result is no expected: %v", result);
            setRetcode(RET_MYSQL_RESULT_INVALID);
            return ;
        }
        ::Cell::GetUserProfileRespMsg* resp = (::Cell::GetUserProfileRespMsg*)m_Response;
        rapidjson::Value& value = dom[0];
        resp->set_phone(value["phone"].GetString());
        resp->set_email(value["email"].GetString());
        resp->set_gender((enum ::Cell::Gender)value["gender"].GetInt());
        resp->set_birthday(value["birthday"].GetString());
        resp->set_avada_url(value["avada_url"].GetString());
        resp->set_avada_size(value["avada_size"].GetString());
        resp->set_usr_name(value["user_name"].GetString());
        resp->set_blood_type((::Cell::BloodType)value["blood_type"].GetUint());
        resp->set_constellation((::Cell::ConstellationType)value["constellation"].GetUint());
        resp->set_zodiac((::Cell::ZodiacType)value["zodiac"].GetUint());
        return ;
    }
    setRetcode(RET_MYSQL_RESULT_INVALID);
}

void UserGetProfProcessor::buildResponse()
{
    ::Cell::GetUserProfileRespMsg* resp = (::Cell::GetUserProfileRespMsg*)m_Response;
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
