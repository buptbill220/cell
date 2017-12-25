#include "ugc/UgcAddCommentProcessor.h"
#include "app_common/app_log.h"
#include "app_common/common_func.h"
#include "app_common/ObjectPallocator.h"
#include "app_common/mysql_common.h"
#include "common/rapidjson/document.h"
#include "proto/ugc.pb.h"
#include <string>

UgcAddCommentProcessor::UgcAddCommentProcessor() : mc_id(0)
{
}

UgcAddCommentProcessor::~UgcAddCommentProcessor()
{
}

bool UgcAddCommentProcessor::init(ngx_http_request_t* r, session_t* s)
{
    ::Cell::AddCommentReqMsg* req = ObjectPallocator< ::Cell::AddCommentReqMsg>::allocate(r->pool);
    m_Response = (void*)ObjectPallocator< ::Cell::AddCommentRespMsg>::allocate(r->pool);
    if (!req->ParseFromArray(s->message_data.data, s->message_data.len)) {
        LOG_ERROR(r->connection->log, "parse AddCommentReqMsg failed");
        setRetcode(RET_INVALID_REQ);
        return false;
    }
    m_Request = (const void*)req;

    //set check user session query
    ngx_str_t redis_query_string;
    redis_query_string.data = (u_char*)ngx_palloc(r->pool, 128);
    u_char* p = ngx_snprintf(redis_query_string.data, 128, "get %D", req->uid());
    redis_query_string.len = p - redis_query_string.data;
    set_redis_query_string(r, redis_query_string);
    LOG_DEBUG(r->connection->log, "set redis_query_string: %V", &redis_query_string);

    //generate time
    struct tm *tmp = localtime((const time_t*)&s->timestamp);
    u_char* add_time = (u_char*)ngx_pcalloc(r->pool, 32);
    ngx_snprintf(add_time, 32, "%04D-%02D-%02D %02D:%02D:%02D",
            tmp->tm_year + 1900, tmp->tm_mon + 1, tmp->tm_mday, tmp->tm_hour, tmp->tm_min, tmp->tm_sec);

    //escape comment
    ngx_str_t escape_content;
    escape_content.data = (u_char*)ngx_palloc(r->pool, req->comment().length() * 2);
    escape_content.len = mysql_escape_string((char*)escape_content.data,
            req->comment().c_str(), req->comment().length());

    //set mysql query string
    ngx_str_t mysql_query_string;
    mysql_query_string.data = (u_char*)ngx_palloc(r->pool, 512);
    p = ngx_snprintf(mysql_query_string.data, 512, "insert into message_comment(msid, mid, uid, org_uid, comment, create_time)"
            " values (%L, %L, %D, %D, '%V', '%s')", req->msid(), req->ugc_id(), req->uid(), req->org_uid(), &escape_content, add_time);
    mysql_query_string.len = p - mysql_query_string.data;
    set_mysql_query_string(r, mysql_query_string);
    
    return BaseProcessor::init(r, s);
}

void UgcAddCommentProcessor::clear()
{
    if (m_Request) {
        ::Cell::AddCommentReqMsg* req = (::Cell::AddCommentReqMsg*)m_Request;
        ObjectPallocator< ::Cell::AddCommentReqMsg>::free(req);
    }
    if (m_Response) {
        ::Cell::AddCommentRespMsg* resp = (::Cell::AddCommentRespMsg*)m_Response;
        ObjectPallocator< ::Cell::AddCommentRespMsg>::free(resp);
    }
}

void UgcAddCommentProcessor::handleMysqlResult(ngx_str_t* result)
{
    if (result->data && result->len) {
        if (result->len == sizeof(ERROR_STRING) - 1 && 
                0 == ngx_strncmp(result->data, ERROR_STRING, result->len)) {
            LOG_ERROR(m_R->connection->log, "mysql insert failed");
            setRetcode(RET_MYSQL_RESULT_INVALID);
            return ;
        }

        rapidjson::Document dom;
        std::string json;
        json.assign((char*)result->data, result->len);
        dom.Parse(json.c_str());
        if (dom.HasParseError()) { 
            LOG_ERROR(m_R->connection->log, "parse mysql insert result error: %V", result);
            setRetcode(RET_MYSQL_RESULT_INVALID);
            return ;
        }

        if (!dom.HasMember("insert_id")) {
            LOG_ERROR(m_R->connection->log, "mysql insert result error: %V", result);
            setRetcode(RET_MYSQL_RESULT_INVALID);
            return ;
        }

        mc_id = dom["insert_id"].GetUint64();
        LOG_DEBUG(m_R->connection->log, "insert message_comment id: %L successfully", mc_id);

        return ;
    }
    setRetcode(RET_MYSQL_RESULT_INVALID);
}

ngx_str_t UgcAddCommentProcessor::getSessionToken() const
{
    ngx_str_t st = ngx_null_string;
    ::Cell::AddCommentReqMsg* req = (::Cell::AddCommentReqMsg*)m_Request;
    st.data = (u_char*)req->session_token().c_str();
    st.len = req->session_token().length();
    return st;
}

void UgcAddCommentProcessor::buildResponse()
{
    ::Cell::AddCommentRespMsg* resp = (::Cell::AddCommentRespMsg*)m_Response;
    switch (m_Retcode) {
        case RET_SUCCESS:
            resp->set_ret_code(::Cell::RC_SUCCESS);
            resp->set_mcid(mc_id);
            break;
        case RET_SESSION_INVALID:
            resp->set_ret_code(::Cell::RC_SESSION_INVALID);
            break;
        default:
            resp->set_ret_code(::Cell::RC_FAILED);
    }
}
