#include "user/UserInitProcessor.h"
#include "proto/common_data.pb.h"
#include "proto/initial.pb.h"
#include "app_common/ObjectPallocator.h"
#include "app_common/app_log.h"
#include "app_common/common_func.h"
#include "common/rapidjson/document.h"
#include <string>
#include <time.h>

UserInitProcessor::UserInitProcessor() : uid(0)
{
}

UserInitProcessor::~UserInitProcessor()
{
}

bool UserInitProcessor::init(ngx_http_request_t* r, session_t* s)
{
    ::Cell::InitReqMsg* req = ObjectPallocator< ::Cell::InitReqMsg>::allocate(r->pool);
    m_Response = (void*)ObjectPallocator< ::Cell::InitRespMsg>::allocate(r->pool);
    if (!req) {
        LOG_ERROR(r->connection->log, "palloc InitReqMsg failed");
        setRetcode(RET_INTERNAL_ERROR);
        return false;
    }
    if (!req->ParseFromArray(s->message_data.data, s->message_data.len)) {
        LOG_ERROR(r->connection->log, "parse InitReqMsg failed");
        setRetcode(RET_INVALID_REQ);
        return false;
    }
    m_Request = (const void*)req;
    //set query sql
    ngx_str_t mysql_query_string;
    mysql_query_string.data = (u_char*)ngx_palloc(r->pool, 256);
    u_char* p_end = ngx_snprintf(mysql_query_string.data, 256, "select uid from user where deviceid = '%s' and phone is NULL limit 1", req->device_id().c_str());
    mysql_query_string.len = p_end - mysql_query_string.data;
    set_mysql_query_string(r, mysql_query_string);
    LOG_DEBUG(r->connection->log, "set mysql query string: %V", &mysql_query_string);
    return BaseProcessor::init(r, s);
}

void UserInitProcessor::clear()
{
    if (m_Request) {
        ::Cell::InitReqMsg* req = (::Cell::InitReqMsg*)m_Request;
        ObjectPallocator< ::Cell::InitReqMsg>::free(req);
    }
    if (m_Response) {
        ::Cell::InitRespMsg* res = (::Cell::InitRespMsg*)m_Response;
        ObjectPallocator< ::Cell::InitRespMsg>::free(res);
    }
}

void UserInitProcessor::handerSqlQueryResult(ngx_str_t* result)
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
            LOG_ERROR(m_R->connection->log, "parse mysql result error:%V", result);
            setRetcode(RET_MYSQL_RESULT_INVALID);
            return ;
        }
        if (!dom.IsArray()) {
            LOG_ERROR(m_R->connection->log, "mysql result:%V is not expected:[{\"1\":1}] or [{empty}]", result);
            setRetcode(RET_MYSQL_RESULT_INVALID);
            return ;
        }
        if (dom.Size() != 0 && dom[0].HasMember("uid")) {
            uid = dom[0]["uid"].GetUint();
            ngx_str_t redis_query_string;
            redis_query_string.data = (u_char*)ngx_palloc(m_R->pool, 128);
            u_char* p = ngx_snprintf(redis_query_string.data, 128, "set %D %V EX %i", 
                    uid, &(m_Session->session_id), m_Session->main_conf->session_timeout);
            redis_query_string.len = p - redis_query_string.data;
            set_redis_query_string(m_R, redis_query_string);
            return ;
        }
        uid = 0;
        ::Cell::InitReqMsg* req = (::Cell::InitReqMsg*)m_Request;
        ngx_str_t mysql_insert_string;
        mysql_insert_string.data = (u_char*)ngx_palloc(m_R->pool, 256);
        u_char* p = ngx_snprintf(mysql_insert_string.data, 256, "insert into user(deviceid) values ('%s')", req->device_id().c_str());
        mysql_insert_string.len = p - mysql_insert_string.data;
        set_mysql_query_string(m_R, mysql_insert_string);
        return ;
    }
    setRetcode(RET_MYSQL_RESULT_INVALID);
}

void UserInitProcessor::handerSqlInsertResult(ngx_str_t* result)
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
            LOG_ERROR(m_R->connection->log, "parse mysql insert result error:%V", result);
            setRetcode(RET_MYSQL_RESULT_INVALID);
            return ;
        }

        if (!dom.IsObject() || !dom.HasMember("insert_id")) {
            LOG_ERROR(m_R->connection->log, "mysql insert result error:%V", result);
            setRetcode(RET_MYSQL_RESULT_INVALID);
            return ;
        }

        //需要根据实际值修改
        uid = dom["insert_id"].GetUint();
        ngx_str_t redis_query_string;
        redis_query_string.data = (u_char*)ngx_palloc(m_R->pool, 128);
        u_char* p = ngx_snprintf(redis_query_string.data, 128, "set %D %V EX %i", 
                uid, &(m_Session->session_id), m_Session->main_conf->session_timeout);
        redis_query_string.len = p - redis_query_string.data;
        set_redis_query_string(m_R, redis_query_string);

        struct tm *tmp = localtime((const time_t*)&m_Session->timestamp);
        u_char add_time[32] = {0};
        u_char *pbuf = ngx_snprintf(add_time, 32, "%04D-%02D-%02D %02D:%02D:%02D", tmp->tm_year + 1900, tmp->tm_mon + 1, tmp->tm_mday, tmp->tm_hour, tmp->tm_min, tmp->tm_sec);
        ngx_str_t mysql_query_string;
        mysql_query_string.data = (u_char*)ngx_palloc(m_R->pool, 128);
        p = ngx_snprintf(mysql_query_string.data, 128, "insert into user_achievement(uid, create_time, update_time)" \
                " values(%D, '%s', '%s')", uid, add_time, add_time);
        mysql_query_string.len = p - mysql_query_string.data;
        set_mysql_query_string(m_R, mysql_query_string);

        return ;
    }
    setRetcode(RET_MYSQL_RESULT_INVALID);
}

void UserInitProcessor::buildResponse()
{
    ::Cell::InitRespMsg* resp = (::Cell::InitRespMsg*)m_Response;
    if (RET_SUCCESS == m_Retcode) {
        resp->set_ret_code(::Cell::RC_SUCCESS);
        resp->set_uid(uid);
        std::string session_token;
        session_token.assign((char*)m_Session->session_id.data, m_Session->session_id.len);
        resp->set_session_token(session_token);
        resp->set_user_stat(::Cell::US_UNREG_USER);
        return ;
    }
    resp->set_ret_code(::Cell::RC_FAILED);
}
