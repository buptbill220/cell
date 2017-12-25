#include "ugc/UgcAddCollectionProcessor.h"
#include "app_common/app_log.h"
#include "app_common/common_func.h"
#include "app_common/ObjectPallocator.h"
#include "app_common/mysql_common.h"
#include "common/rapidjson/document.h"
#include "proto/ugc.pb.h"
#include <string>

UgcAddCollectionProcessor::UgcAddCollectionProcessor() : clt_id(0)
{
}

UgcAddCollectionProcessor::~UgcAddCollectionProcessor()
{
}

bool UgcAddCollectionProcessor::init(ngx_http_request_t* r, session_t* s)
{
    ::Cell::AddCollectionReqMsg* req = ObjectPallocator< ::Cell::AddCollectionReqMsg>::allocate(r->pool);
    m_Response = (void*)ObjectPallocator< ::Cell::AddCollectionRespMsg>::allocate(r->pool);
    if (!req->ParseFromArray(s->message_data.data, s->message_data.len)) {
        LOG_ERROR(r->connection->log, "parse AddCollectionReqMsg failed");
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
    LOG_DEBUG(r->connection->log, "set redis_query_string: %V",&redis_query_string);

    //set mysql query string
    ngx_str_t mysql_query_string;
    mysql_query_string.data = (u_char*)ngx_palloc(r->pool, 512);
    p = ngx_snprintf(mysql_query_string.data, 512, "select clt_id, status from message_collection where uid = %D and mid = %L", 
            req->uid(), req->ugc_id_list().ugc_id());
    mysql_query_string.len = p - mysql_query_string.data;
    set_mysql_query_string(r, mysql_query_string);

    return BaseProcessor::init(r, s);
}

void UgcAddCollectionProcessor::clear()
{
    if (m_Request) {
        ::Cell::AddCollectionReqMsg* req = (::Cell::AddCollectionReqMsg*)m_Request;
        ObjectPallocator< ::Cell::AddCollectionReqMsg>::free(req);
    }
    if (m_Response) {
        ::Cell::AddCollectionRespMsg* resp = (::Cell::AddCollectionRespMsg*)m_Response;
        ObjectPallocator< ::Cell::AddCollectionRespMsg>::free(resp);
    }
}

void UgcAddCollectionProcessor::handleQueryCltResult(ngx_str_t* result)
{
    if (result->data && result->len) {
        if (result->len == sizeof(ERROR_STRING) - 1 && 
                0 == ngx_strncmp(result->data, ERROR_STRING, result->len)) {
            LOG_ERROR(m_R->connection->log, "mysql query failed");
            setRetcode(RET_MYSQL_RESULT_INVALID);
            return ;
        }
        
        rapidjson::Document dom;
        std::string json;
        json.assign((char*)result->data, result->len);
        dom.Parse(json.c_str());
        if (dom.HasParseError()) {
            LOG_ERROR(m_R->connection->log, "parse mysql query result error: %V", result);
            setRetcode(RET_MYSQL_RESULT_INVALID);
            return ;
        }

        if (!dom.IsArray()) {
            LOG_ERROR(m_R->connection->log, "parse mysql queryresult error: %V", result);
            setRetcode(RET_MYSQL_RESULT_INVALID);
            return ;
        }

        ::Cell::AddCollectionReqMsg* req = (::Cell::AddCollectionReqMsg*)m_Request;
        ngx_str_t mysql_query_string;
        u_char* p = NULL;

        //generate time
        struct tm *tmp = localtime((const time_t*)&m_Session->timestamp);
        u_char* add_time = (u_char*)ngx_pcalloc(m_R->pool, 32);
        ngx_snprintf(add_time, 32, "%04D-%02D-%02D %02D:%02D:%02D",
                tmp->tm_year + 1900, tmp->tm_mon + 1, tmp->tm_mday, tmp->tm_hour, tmp->tm_min, tmp->tm_sec);

        const ::Cell::UGCIdList& ugc_id_list = req->ugc_id_list();
        if (dom.Size() == 0) {
            if (::Cell::OK == req->confirmtype()) {
                mysql_query_string.data = (u_char*)ngx_palloc(m_R->pool, 512);
                p = ngx_snprintf(mysql_query_string.data, 512, "insert into message_collection"
                        "(uid, org_uid, mid, msid, mstid, create_time, modify_time) values"
                        "(%D, %D, %L, %L, %L, '%s', '%s')", req->uid(), ugc_id_list.org_uid(), ugc_id_list.ugc_id(), 
                        ugc_id_list.msid(), ugc_id_list.mstid(), add_time, add_time);
            } else {
                LOG_ERROR(m_R->connection->log, "uid %D haven't collect mid %L", req->uid(), ugc_id_list.ugc_id());
                return ;
            }
        } else {
            uint32_t status = dom[0]["status"].GetUint();
            clt_id = dom[0]["clt_id"].GetUint64();
            if (status == 1 && ::Cell::CANCEL == req->confirmtype()) {
                mysql_query_string.data = (u_char*)ngx_palloc(m_R->pool, 512);
                p = ngx_snprintf(mysql_query_string.data, 512, "update message_collection set modify_time = '%s', status = 2 where clt_id = %L", add_time, clt_id);
            } else if (status == 2 && ::Cell::OK == req->confirmtype()) {
                mysql_query_string.data = (u_char*)ngx_palloc(m_R->pool, 512);
                p = ngx_snprintf(mysql_query_string.data, 512, "update message_collection set modify_time = '%s', status = 1 where clt_id = %L", add_time, clt_id);
            } else {
                LOG_ERROR(m_R->connection->log, "uid %D haven't collect mid %L cann't be cancel or has collected cann't be collect", 
                        req->uid(), ugc_id_list.ugc_id());
                return ;
            }
        }
        setData("status", (void*)"ok");
        mysql_query_string.len = p - mysql_query_string.data;
        set_mysql_query_string(m_R, mysql_query_string);

        return ;
    }
    setRetcode(RET_MYSQL_RESULT_INVALID);
}

void UgcAddCollectionProcessor::handleAddCltResult(ngx_str_t* result)
{
    if (result->data && result->len) {
        if (result->len == sizeof(ERROR_STRING) - 1 && 
                0 == ngx_strncmp(result->data, ERROR_STRING, result->len)) {
            LOG_ERROR(m_R->connection->log, "mysql add failed");
            setRetcode(RET_MYSQL_RESULT_INVALID);
            return ;
        }
        
        rapidjson::Document dom;
        std::string json;
        json.assign((char*)result->data, result->len);
        dom.Parse(json.c_str());
        if (dom.HasParseError()) {
            LOG_ERROR(m_R->connection->log, "parse mysql add result error: %V", result);
            setRetcode(RET_MYSQL_RESULT_INVALID);
            return ;
        }

        if (!dom.IsObject()) {
            LOG_ERROR(m_R->connection->log, "parse mysql insert result error: %V", result);
            setRetcode(RET_MYSQL_RESULT_INVALID);
            return ;
        }
        if (dom.HasMember("insert_id")) {
            clt_id = dom["insert_id"].GetUint64();
        }
        LOG_DEBUG(m_R->connection->log, "add message_collection:%L successfully", clt_id);
        return ;
    }
    setRetcode(RET_MYSQL_RESULT_INVALID);
}

ngx_str_t UgcAddCollectionProcessor::getSessionToken() const
{
    ngx_str_t st = ngx_null_string;
    ::Cell::AddCollectionReqMsg* req = (::Cell::AddCollectionReqMsg*)m_Request;
    st.data = (u_char*)req->session_token().c_str();
    st.len = req->session_token().length();
    return st;
}

void UgcAddCollectionProcessor::buildResponse()
{
    ::Cell::AddCollectionRespMsg* resp = (::Cell::AddCollectionRespMsg*)m_Response;
    switch (m_Retcode) {
        case RET_SUCCESS:
            resp->set_ret_code(::Cell::RC_SUCCESS);
            resp->set_clt_id(clt_id);
            break;
        case RET_SESSION_INVALID:
            resp->set_ret_code(::Cell::RC_SESSION_INVALID);
            break;
        default:
            resp->set_ret_code(::Cell::RC_FAILED);
    }
}
