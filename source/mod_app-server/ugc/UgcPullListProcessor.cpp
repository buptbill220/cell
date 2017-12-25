#include "ugc/UgcPullListProcessor.h"
#include "app_common/app_log.h"
#include "app_common/common_func.h"
#include "app_common/ObjectPallocator.h"
#include "common/rapidjson/document.h"
#include "common/geohash.h"
#include "proto/common_data.pb.h"
#include "proto/ugc.pb.h"
#include <string>

#define UGC_LIST_TYPE_LOC   "local"
#define UGC_LIST_TYPE_NEI   "neighbor"
#define UGC_LIST_TYPE_FAR   "far"

UgcPullListProcessor::UgcPullListProcessor() :
    m_Neighbors(NULL), 
    m_Geohash(NULL)
{
}

UgcPullListProcessor::~UgcPullListProcessor()
{
}

bool UgcPullListProcessor::init(ngx_http_request_t* r, session_t* s)
{
    ::Cell::PullUGCReqMsg* req = ObjectPallocator< ::Cell::PullUGCReqMsg>::allocate(r->pool);
    m_Response = (void*)ObjectPallocator< ::Cell::PullUGCRespMsg>::allocate(r->pool);
    if (!req->ParseFromArray(s->message_data.data, s->message_data.len)) {
        LOG_ERROR(r->connection->log, "parse PullUGCReqMsg failed");
        setRetcode(RET_INVALID_REQ);
        return false;
    }
    m_Request = (const void*)req;

    // set check session query
    ngx_str_t redis_query_string;
    redis_query_string.data = (u_char*)ngx_palloc(r->pool, 128);
    u_char* p = ngx_snprintf(redis_query_string.data, 128, "get %D", req->uid());
    redis_query_string.len = p - redis_query_string.data;
    set_redis_query_string(r, redis_query_string);

    return BaseProcessor::init(r, s);
}

void UgcPullListProcessor::clear()
{
    if (m_Request) {
        ::Cell::PullUGCReqMsg* req = (::Cell::PullUGCReqMsg*)m_Request;
        ObjectPallocator< ::Cell::PullUGCReqMsg>::free(req);
    }
    if (m_Response) {
        ::Cell::PullUGCRespMsg* resp = (::Cell::PullUGCRespMsg*)m_Response;
        ObjectPallocator< ::Cell::PullUGCRespMsg>::free(resp);
    }
}

void UgcPullListProcessor::handerUgcType(ngx_str_t* type)
{
    ::Cell::PullUGCReqMsg* req = (::Cell::PullUGCReqMsg*)m_Request;
    int need_num = req->need_num() > 0 ? req->need_num() : 5;
    need_num = need_num >= 5 ? 5 : need_num;

    ngx_str_t mysql_query_string;
    u_char* p = NULL;
    mysql_query_string.data = (u_char*)ngx_palloc(m_R->pool, 1024);

    if (NULL == m_Geohash) {
        ::Cell::Geo geo = req->geo();
        m_Geohash = geohash_encode(m_R->pool, geo.latitude(), geo.longitude(), 5);
        if (NULL == m_Geohash) {
            setRetcode(RET_INVALID_REQ);
            p = mysql_query_string.data;
            mysql_query_string.len = 0;
            set_mysql_query_string(m_R, mysql_query_string);
            return ;
        }
        m_Neighbors = geohash_neighbors(m_R->pool, m_Geohash);
    }

    u_char forbid_uids[256] = {0};
    ngx_str_t tmp_uids;
    tmp_uids.data = forbid_uids;
    p = ngx_snprintf(tmp_uids.data, 256, "(%D", req->uid());
    if (req->pull_type() == ::Cell::PT_PLAY && req->forbid_uid_size() > 0) {
        for (int i = 0; i <= 17 && i < req->forbid_uid_size(); ++i) {
            int len = p - tmp_uids.data;
            p = ngx_snprintf(p, 256 - len, ",%D", req->forbid_uid(i));
        }
    }
    *p++ = ')';
    tmp_uids.len = p - tmp_uids.data;
    if (type->len == sizeof(UGC_LIST_TYPE_LOC) - 1 && 
            0 == ngx_strncmp(type->data, UGC_LIST_TYPE_LOC, sizeof(UGC_LIST_TYPE_LOC) - 1)) {
        p = ngx_snprintf(mysql_query_string.data, 1024, "select msid, mid, uid from message_seed where geohash = '%s' and "
                "uid not in %V and mid > %L and alive > 0 and mid not in (select mid from (select mid from msg_blacklist_%D "
                "where uid = %D order by bid desc limit 100) as t) order by mid asc limit %D", m_Geohash, &tmp_uids, 
                req->last_id(), req->uid() & BLACKLIST_TABLE_MASK, req->uid(), need_num);
    } else if (type->len == sizeof(UGC_LIST_TYPE_NEI) - 1 && 
            0 == ngx_strncmp(type->data, UGC_LIST_TYPE_NEI, sizeof(UGC_LIST_TYPE_NEI) - 1)) {
        p = ngx_snprintf(mysql_query_string.data, 1024, "select msid, mid, uid from message_seed where geohash in "
                "('%s', '%s', '%s', '%s', '%s', '%s','%s', '%s') and uid not in %V and mid > %L and alive > 0 and mid "
                "not in (select mid from (select mid from msg_blacklist_%D where uid = %D order by bid desc limit 100)"
                "as t) order by mid asc limit %D", m_Neighbors[0], m_Neighbors[1], m_Neighbors[2], m_Neighbors[3], 
                m_Neighbors[4], m_Neighbors[5], m_Neighbors[6], m_Neighbors[7], &tmp_uids, req->last_id(), 
                req->uid() & BLACKLIST_TABLE_MASK, req->uid(), need_num);
    } else {
        p = ngx_snprintf(mysql_query_string.data, 1024, "select msid, mid, uid from message_seed where uid not in %V and mid > %L "
                "and alive > 0 and mid not in (select mid from (select mid from msg_blacklist_%D where uid = %D order by bid "
                "desc limit 100) as t) order by mid asc limit %D", &tmp_uids, req->last_id(), 
                req->uid() & BLACKLIST_TABLE_MASK, req->uid(), need_num);
    }
    mysql_query_string.len = p - mysql_query_string.data;
    set_mysql_query_string(m_R, mysql_query_string);
}

void UgcPullListProcessor::handerUgcMysqlResult(ngx_str_t* result)
{
    if (result->data && result->len) {
        setRetcode(RET_SUCCESS);
        if (result->len == sizeof(ERROR_STRING) - 1 && 
                0 == ngx_strncmp(result->data, ERROR_STRING, result->len)) {
            LOG_ERROR(m_R->connection->log, "mysql query error in handerUgcMysqlResult");
            setRetcode(RET_MYSQL_RESULT_INVALID);
            return ;
        }
        
        std::string json;
        json.assign((char*)result->data, result->len);
        rapidjson::Document dom;
        dom.Parse(json.c_str());

        if (dom.HasParseError() || !dom.IsArray()) {
            LOG_ERROR(m_R->connection->log, "parse mysql result json failed in handerUgcMysqlResult: %V", result);
            setRetcode(RET_MYSQL_RESULT_INVALID);
            return ;
        }

        if (0 == dom.Size()) {
            LOG_DEBUG(m_R->connection->log, "query noting in handerUgcMysqlResult");
            return ;
        }

        ::Cell::PullUGCRespMsg* resp = (::Cell::PullUGCRespMsg*)m_Response;
        setData("ugc_status", (void*)OK_STRING);
        for (int i = 0; i < dom.Size(); ++i) {
            rapidjson::Value& value = dom[i];
            ::Cell::UGCIdList* ugc_list = resp->add_ugc_id_list();
            ugc_list->set_ugc_id(value["mid"].GetUint());
            ugc_list->set_org_uid(value["uid"].GetUint());
            ugc_list->set_msid(value["msid"].GetUint64());
        }
        return ;
    }
    setRetcode(RET_MYSQL_RESULT_INVALID);
}

ngx_str_t UgcPullListProcessor::getSessionToken() const
{
    ngx_str_t st = ngx_null_string;
    ::Cell::PullUGCReqMsg* req = (::Cell::PullUGCReqMsg*)m_Request;
    st.data = (u_char*)req->session_token().c_str();
    st.len = req->session_token().length();
    return st;
}

void UgcPullListProcessor::buildResponse()
{
    ::Cell::PullUGCRespMsg* resp = (::Cell::PullUGCRespMsg*)m_Response;
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
