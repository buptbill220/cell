#include "user/UserLoginProcessor.h"
#include "app_common/ObjectPallocator.h"
#include "app_common/app_log.h"
#include "app_common/common_func.h"
#include "proto/common_data.pb.h"
#include "proto/initial.pb.h"
#include "common/rapidjson/document.h"
#include <string>

UserLoginProcessor::UserLoginProcessor()
{
}

UserLoginProcessor::~UserLoginProcessor()
{
}

bool UserLoginProcessor::init(ngx_http_request_t* r, session_t* s)
{
    ::Cell::LoginReqMsg* req = ObjectPallocator< ::Cell::LoginReqMsg>::allocate(r->pool);
    m_Response = (void*)ObjectPallocator< ::Cell::LoginRespMsg>::allocate(r->pool);
    if (!req->ParseFromArray(s->message_data.data, s->message_data.len)) {
        LOG_ERROR(r->connection->log, "parse LoginReqMsg failed");
        setRetcode(RET_INVALID_REQ);
        return false;
    }

    m_Request = (const void*)req;
    ngx_str_t mysql_query_string;
    mysql_query_string.data = (u_char*)ngx_palloc(r->pool, 256);
    u_char* p = ngx_snprintf(mysql_query_string.data, 256, "select uid, passwd from user where phone  = '%s'", req->usr_info().c_str());
    mysql_query_string.len = p - mysql_query_string.data;
    set_mysql_query_string(r, mysql_query_string);
    LOG_DEBUG(r->connection->log, "set mysql_query_string: %V", &mysql_query_string);

    return BaseProcessor::init(r, s);
}

void UserLoginProcessor::clear()
{
    if (m_Request) {
        ::Cell::LoginReqMsg* req = (::Cell::LoginReqMsg*)m_Request;
        ObjectPallocator< ::Cell::LoginReqMsg>::free(req);
    }
    if (m_Response) {
        ::Cell::LoginRespMsg* resp = (::Cell::LoginRespMsg*)m_Response;
        ObjectPallocator< ::Cell::LoginRespMsg>::free(resp);
    }
}

void UserLoginProcessor::handerSqlQueryResult(ngx_str_t* result)
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
        if (dom.HasParseError()) {
            LOG_ERROR(m_R->connection->log, "json parse error, json: %V", result);
            setRetcode(RET_MYSQL_RESULT_INVALID);
            return ;
        }
        if (!dom.IsArray() || dom.Size() != 1 || !dom[0].HasMember("passwd")) {
            LOG_ERROR(m_R->connection->log, "mysql result is not expected: %V", result);
            setRetcode(RET_MYSQL_RESULT_INVALID);
            return ;
        }
        ::Cell::LoginReqMsg* req = (::Cell::LoginReqMsg*)m_Request;
        const u_char* passwd = (const u_char*)dom[0]["passwd"].GetString();
        if (0 != ngx_strncmp(passwd, req->password().c_str(), req->password().length())) {
            LOG_ERROR(m_R->connection->log, "passwd: %s is not expected: %s", req->password().c_str(), passwd);
            setRetcode(RET_MYSQL_RESULT_INVALID);
            return ;
        }
        // login successfully
        ngx_uint_t* puid = (ngx_uint_t*)ngx_palloc(m_R->pool, sizeof(ngx_uint_t));
        *puid = dom[0]["uid"].GetUint();
        setData("uid", (void*)puid);
        ngx_str_t redis_query_string;
        redis_query_string.data = (u_char*)ngx_palloc(m_R->pool, 128);
        u_char* p = ngx_snprintf(redis_query_string.data, 128, "set %D %V EX %i", *puid, &(m_Session->session_id), m_Session->main_conf->session_timeout);
        redis_query_string.len = p - redis_query_string.data;
        set_redis_query_string(m_R, redis_query_string);

        // update user geo
        ngx_str_t mysql_query_string;
        mysql_query_string.data = (u_char*)ngx_palloc(m_R->pool, 128);
        p = ngx_snprintf(mysql_query_string.data, 128, "update user set latitude = %.6f, longitude = %.6f where uid = %D", req->geo().latitude(), req->geo().longitude(), *puid);
        mysql_query_string.len = p - mysql_query_string.data;
        set_mysql_query_string(m_R, mysql_query_string);

        return ;
    }
    setRetcode(RET_MYSQL_RESULT_INVALID);
}

void UserLoginProcessor::buildResponse()
{
    ::Cell::LoginRespMsg* resp = (::Cell::LoginRespMsg*)m_Response;
    if (RET_SUCCESS == m_Retcode) {
        resp->set_ret_code(::Cell::RC_SUCCESS);
        resp->set_uid(*(ngx_uint_t*)getData("uid"));
        std::string session_token;
        session_token.assign((char*)m_Session->session_id.data, m_Session->session_id.len);
        resp->set_session_token(session_token);
        resp->set_user_stat(::Cell::US_REG_USER);
        return ;
    }
    resp->set_ret_code(::Cell::RC_FAILED);
}
