#include "user/UserGetUgcListProcessor.h"
#include "app_common/common_func.h"
#include "app_common/ObjectPallocator.h"
#include "app_common/app_log.h"
#include "proto/common_data.pb.h"
#include "proto/ugc.pb.h"
#include "common/rapidjson/document.h"
#include <string>

UserGetUgcListProcessor::UserGetUgcListProcessor()
{
}

UserGetUgcListProcessor::~UserGetUgcListProcessor()
{
}

bool UserGetUgcListProcessor::init(ngx_http_request_t* r, session_t* s)
{
    ::Cell::GetUGCListReqMsg* req = ObjectPallocator< ::Cell::GetUGCListReqMsg>::allocate(r->pool);
    m_Response = (void*)ObjectPallocator< ::Cell::GetUGCListRespMsg>::allocate(r->pool);
    if (!req->ParseFromArray(s->message_data.data, s->message_data.len)) {
        LOG_ERROR(r->connection->log, "parse GetUGCListReqMsg failed");
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

    //set query ugc list query string
    ngx_str_t mysql_query_string;
    mysql_query_string.data = (u_char*)ngx_palloc(r->pool, 512);
    p = NULL;
    const char *prefix = NULL;
    char where1[256] = {0}, where2[128] = {0};

    switch (req->ugc_list_type()) {
        case ::Cell::UGC_GEN:
            prefix = "select mid from message ";
            snprintf(where1, 256, "where uid = %u ", req->org_uid());
            if (req->last_id() > 0) {
                snprintf(where2, 128, "and mid < %u order by mid desc limit %u", req->last_id(), req->need_ugc_num());
            } else {
                snprintf(where2, 128, "order by mid desc limit %u", req->need_ugc_num());
            }
            break;
        case ::Cell::UGC_POST:
            prefix = "select mid, org_uid, msid, mstid from message_post ";
            snprintf(where1, 256, "where to_uid = %u ", req->org_uid());
            if (req->last_id() > 0) {
                snprintf(where2, 128, "and mstid < %u order by mstid desc limit %u", req->last_id(), req->need_ugc_num());
            } else {
                snprintf(where2, 128, "order by mstid desc limit %u", req->need_ugc_num());
            }
            break;
        case ::Cell::UGC_COLLECT:
            prefix = "select mid, org_uid, msid, mstid from message_collection ";
            snprintf(where1, 256, "where uid = %u ", req->uid());
            if (req->last_id() > 0) {
                snprintf(where2, 128, "and clt_id < %u order by clt_id desc limit %u", req->last_id(), req->need_ugc_num());
            } else {
                snprintf(where2, 128, "order by clt_id desc limit %u", req->need_ugc_num());
            }
            break;
        default:
            prefix = "select mid, org_uid, msid, mstid from message_pass ";
            snprintf(where1, 256, "where to_uid = %u ", req->org_uid());
            if (req->last_id() > 0) {
                snprintf(where2, 128, "and mstid < %u order by mstid desc limit %u", req->last_id(), req->need_ugc_num());
            } else {
                snprintf(where2, 128, "order by mstid desc limit %u", req->need_ugc_num());
            }
    }


    p = ngx_snprintf(mysql_query_string.data, 512, "%s %s %s", prefix, where1, where2); 
    mysql_query_string.len = p - mysql_query_string.data;
    set_mysql_query_string(r, mysql_query_string);

    return BaseProcessor::init(r, s);
}

void UserGetUgcListProcessor::clear()
{
    if (m_Request) {
        ::Cell::GetUGCListReqMsg* req = (::Cell::GetUGCListReqMsg*)m_Request;
        ObjectPallocator< ::Cell::GetUGCListReqMsg>::free(req);
    }
    if (m_Response) {
        ::Cell::GetUGCListRespMsg* resp = (::Cell::GetUGCListRespMsg*)m_Response;
        ObjectPallocator< ::Cell::GetUGCListRespMsg>::free(resp);
    }
}

ngx_str_t UserGetUgcListProcessor::getSessionToken() const
{
    ngx_str_t st = ngx_null_string;
    ::Cell::GetUGCListReqMsg* req = (::Cell::GetUGCListReqMsg*)m_Request;
    st.data = (u_char*)req->session_token().c_str();
    st.len = req->session_token().length();
    return st;
}

void UserGetUgcListProcessor::handerUgcListResult(ngx_str_t* result)
{
    if (result->data && result->len) {
        if (result->len == sizeof(ERROR_STRING) - 1 && 
                0 == ngx_strncmp(result->data, ERROR_STRING, result->len)) {
            LOG_ERROR(m_R->connection->log, "get ugc list result error");
            setRetcode(RET_MYSQL_RESULT_INVALID);
            return ;
        }
        rapidjson::Document dom;
        std::string json;
        json.assign((char*)result->data, result->len);
        dom.Parse(json.c_str());
        if (dom.HasParseError()) {
            LOG_ERROR(m_R->connection->log, "parse ugc list result json failed");
            setRetcode(RET_MYSQL_RESULT_INVALID);
            return ;
        }
        if (!dom.IsArray()) {
            LOG_ERROR(m_R->connection->log, "ugc list json is invalid: %V", result);
            setRetcode(RET_MYSQL_RESULT_INVALID);
            return ;
        }
        if (dom.Size() == 0) {
            return ;
        }
        ::Cell::GetUGCListRespMsg* resp = (::Cell::GetUGCListRespMsg*)m_Response;
        bool ugc_self = !(dom[0].HasMember("msid"));
        for (size_t i = 0; i < dom.Size(); ++i) {
            rapidjson::Value& value = dom[i];
            ::Cell::UGCIdList* list = resp->add_ugc_id_list();
            if (ugc_self) {
                list->set_ugc_id(value["mid"].GetUint64());
            } else {
                list->set_ugc_id(value["mid"].GetUint64());
                list->set_org_uid(value["org_uid"].GetUint());
                list->set_msid(value["msid"].GetUint());
                list->set_mstid(value["mstid"].GetUint());
            }
        }
        return ;
    }
    setRetcode(RET_MYSQL_RESULT_INVALID);
}

void UserGetUgcListProcessor::buildResponse()
{
    ::Cell::GetUGCListRespMsg* resp = (::Cell::GetUGCListRespMsg*)m_Response;
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
