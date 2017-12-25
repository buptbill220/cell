#include "ugc/UgcGetCommentProcessor.h"
#include "app_common/app_log.h"
#include "app_common/common_func.h"
#include "app_common/ObjectPallocator.h"
#include "app_common/mysql_common.h"
#include "common/rapidjson/document.h"
#include "proto/common_data.pb.h"
#include "proto/ugc.pb.h"
#include <string>

UgcGetCommentProcessor::UgcGetCommentProcessor()
{
}

UgcGetCommentProcessor::~UgcGetCommentProcessor()
{
}

bool UgcGetCommentProcessor::init(ngx_http_request_t* r, session_t* s)
{
    ::Cell::GetCommentReqMsg* req = ObjectPallocator< ::Cell::GetCommentReqMsg>::allocate(r->pool);
    m_Response = (void*)ObjectPallocator< ::Cell::GetCommentRespMsg>::allocate(r->pool);
    if (!req->ParseFromArray(s->message_data.data, s->message_data.len)) {
        LOG_ERROR(r->connection->log, "parse GetCommentReqMsg failed");
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

    //set mysql query string
    ngx_str_t mysql_query_string;
    mysql_query_string.data = (u_char*)ngx_palloc(r->pool, 512);
    
    const char* prefix = "select mcid, message_comment.uid as uid, user_name, avada_url, comment, message_comment.create_time as create_time from "
        "message_comment inner join user on user.uid = message_comment.uid ";
    int need_num = req->need_num();

    char where1[128] = {0}, where2[128] = {0};

    switch (req->comment_type()) {
        case ::Cell::USER_POST_TYPE:
            snprintf(where1, 128, "where message_comment.uid = %lu ", req->cid());
            break;
        case ::Cell::USER_RECV_TYPE:
            snprintf(where1, 128, "where org_uid = %lu ", req->cid());
            break;
        case ::Cell::MSG_TYPE:
            snprintf(where1, 128, "where mid = %lu ", req->cid());
            break;
    }
    if (req->last_id() > 0) {
        snprintf(where2, 128, "and mcid > %lu order by mcid asc limit %u", req->last_id(), need_num);
    } else {
        snprintf(where2, 128, "order by mcid asc limit %u", need_num);
    }

    p = ngx_snprintf(mysql_query_string.data, 512, "%s %s %s", prefix, where1, where2);
    mysql_query_string.len = p - mysql_query_string.data;
    set_mysql_query_string(r, mysql_query_string);

    return BaseProcessor::init(r, s);
}

void UgcGetCommentProcessor::clear()
{
    if (m_Request) {
        ::Cell::GetCommentReqMsg* req = (::Cell::GetCommentReqMsg*)m_Request;
        ObjectPallocator< ::Cell::GetCommentReqMsg>::free(req);
    }
    if (m_Response) {
        ::Cell::GetCommentRespMsg* resp = (::Cell::GetCommentRespMsg*)m_Response;
        ObjectPallocator< ::Cell::GetCommentRespMsg>::free(resp);
    }
}

void UgcGetCommentProcessor::handerMysqlResult(ngx_str_t* result)
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
        if (dom.HasParseError() || !dom.IsArray()) { 
            LOG_ERROR(m_R->connection->log, "parse mysql query result error: %V", result);
            setRetcode(RET_MYSQL_RESULT_INVALID);
            return ;
        }

        if (0 == dom.Size()) {
            LOG_DEBUG(m_R->connection->log, "query nothing in handerMysqlResult");
            return ;
        }

        ::Cell::GetCommentRespMsg* resp = (::Cell::GetCommentRespMsg*)m_Response;
        for (int i = 0; i < dom.Size(); ++i) {
            rapidjson::Value& value = dom[i];
            ::Cell::GetCommentRespMsg_CommentDetail* com_det = resp->add_comments();
            com_det->set_mcid(value["mcid"].GetUint64());
            com_det->set_uid(value["uid"].GetUint());
            com_det->set_name(value["user_name"].GetString());
            com_det->set_avada_url(value["avada_url"].GetString());
            com_det->set_comment(value["comment"].GetString());
            com_det->set_create_time(value["create_time"].GetString());
        }
        return ;
    }
    setRetcode(RET_MYSQL_RESULT_INVALID);
}

ngx_str_t UgcGetCommentProcessor::getSessionToken() const
{
    ngx_str_t st = ngx_null_string;
    ::Cell::GetCommentReqMsg* req = (::Cell::GetCommentReqMsg*)m_Request;
    st.data = (u_char*)req->session_token().c_str();
    st.len = req->session_token().length();
    return st;
}

void UgcGetCommentProcessor::buildResponse()
{
    ::Cell::GetCommentRespMsg* resp = (::Cell::GetCommentRespMsg*)m_Response;
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
