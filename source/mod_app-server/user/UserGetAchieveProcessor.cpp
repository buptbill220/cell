#include "user/UserGetAchieveProcessor.h"
#include "app_common/common_func.h"
#include "app_common/app_log.h"
#include "app_common/ObjectPallocator.h"
#include "proto/common_data.pb.h"
#include "proto/account.pb.h"
#include "common/rapidjson/document.h"
#include <string>

UserGetAchieveProcessor::UserGetAchieveProcessor()
{
}

UserGetAchieveProcessor::~UserGetAchieveProcessor()
{
}

bool UserGetAchieveProcessor::init(ngx_http_request_t* r, session_t* s)
{
    ::Cell::GetUserProfileReqMsg* req = ObjectPallocator< ::Cell::GetUserProfileReqMsg>::allocate(r->pool);
    m_Response = (void*)ObjectPallocator< ::Cell::GetUserAchieveRespMsg>::allocate(r->pool);
    if (!req->ParseFromArray(s->message_data.data, s->message_data.len)) {
        LOG_ERROR(r->connection->log, "parse GetUserProfileReqMsg failed");
        setRetcode(RET_INVALID_REQ);
        return false;
    }
    if (::Cell::US_UGC_ACHIEVE != req->usr_prf_type() && ::Cell::US_PK_ACHIEVE != req->usr_prf_type()) {
        LOG_ERROR(r->connection->log, "GetUserProfileReqMsg type is not US_ACHIEVE, but is: %d", req->usr_prf_type());
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
    LOG_DEBUG(r->connection->log, "set redis_query_string: %V", &redis_query_string);

    return BaseProcessor::init(r, s);
}

void UserGetAchieveProcessor::handerCacheAchieve()
{
    ::Cell::GetUserProfileReqMsg* req = (::Cell::GetUserProfileReqMsg*)m_Request;
    ngx_str_t redis_query_string;
    redis_query_string.data = (u_char*)ngx_palloc(m_R->pool, 32);
    u_char* p = ngx_snprintf(redis_query_string.data, 32, "get %D_a", req->org_uid());
    redis_query_string.len = p - redis_query_string.data;
    set_redis_query_string(m_R, redis_query_string);
}

void UserGetAchieveProcessor::clear()
{
    if (m_Request) {
        ::Cell::GetUserProfileReqMsg* req = (::Cell::GetUserProfileReqMsg*)m_Request;
        ObjectPallocator< ::Cell::GetUserProfileReqMsg>::free(req);
    }
    if (m_Response) {
        ::Cell::GetUserAchieveRespMsg* resp = (::Cell::GetUserAchieveRespMsg*)m_Response;
        ObjectPallocator< ::Cell::GetUserAchieveRespMsg>::free(resp);
    }
}

ngx_str_t UserGetAchieveProcessor::getSessionToken() const
{
    ngx_str_t st = ngx_null_string;
    ::Cell::GetUserProfileReqMsg* req = (::Cell::GetUserProfileReqMsg*)m_Request;
    st.data = (u_char*)req->session_token().c_str();
    st.len = req->session_token().length();
    return st;
}

void UserGetAchieveProcessor::handerAchieveResult(ngx_str_t* result)
{
    if (result->data && result->len) {
        void* redis_status = getData("redis_status");
        if (result->len == sizeof(ERROR_STRING) - 1 && 
                0 == ngx_strncmp(result->data, ERROR_STRING, result->len)) {
            if (redis_status) {
                LOG_ERROR(m_R->connection->log, "get achieve mysql result error");
                setRetcode(RET_MYSQL_RESULT_INVALID);
            } else {
                LOG_ERROR(m_R->connection->log, "get achieve redis result failed");
                setRetcode(RET_REDIS_RESULT_INVALID);
            }
            return ;
        }
        rapidjson::Document dom;
        std::string json;
        json.assign((char*)result->data, result->len);
        dom.Parse(json.c_str());
        if (dom.HasParseError()) {
            if (redis_status) {
                LOG_ERROR(m_R->connection->log, "parse achieve result json error");
                setRetcode(RET_MYSQL_RESULT_INVALID);
            } else {
                LOG_DEBUG(m_R->connection->log, "parse achieve result: %V failed", result);
                setRetcode(RET_REDIS_RESULT_INVALID);
            }
            return ;
        }
        if (!dom.IsArray() || dom.Size() != 1) {
            if (redis_status) {
                LOG_ERROR(m_R->connection->log, "achieve json result is invalid: %V", result);
                setRetcode(RET_MYSQL_RESULT_INVALID);
            } else {
                LOG_ERROR(m_R->connection->log, "achive json result is invalid in redis: %V", result);
                setRetcode(RET_REDIS_RESULT_INVALID);
            }
            return ;
        }
        setRetcode(RET_SUCCESS);
        ::Cell::GetUserAchieveRespMsg* resp = (::Cell::GetUserAchieveRespMsg*)m_Response;
        rapidjson::Value& value = dom[0];
        resp->set_create_ugc_num(value["total_create"].GetUint());
        resp->set_post_ugc_num(value["total_post_num"].GetUint());
        resp->set_pass_ugc_num(value["total_pass_num"].GetUint());
        resp->set_total_post_dist((uint32_t)value["total_dist"].GetDouble());
        resp->set_max_post_dist((uint32_t)value["max_dist"].GetDouble());
        resp->set_ugc_seed_num(value["total_alive"].GetUint());
        resp->set_pk_coin(value["pk_coin"].GetInt64());
        resp->set_pk_win_num(value["pk_win_num"].GetUint());
        resp->set_pk_lose_num(value["pk_lose_num"].GetUint());
        resp->set_pk_rank(value["pk_rank"].GetUint());
        return ;
    }
    setRetcode(RET_MYSQL_RESULT_INVALID);
}

void UserGetAchieveProcessor::buildResponse()
{
    ::Cell::GetUserAchieveRespMsg* resp = (::Cell::GetUserAchieveRespMsg*)m_Response;
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
