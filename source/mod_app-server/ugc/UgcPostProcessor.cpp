#include "ugc/UgcPostProcessor.h"
#include "app_common/app_log.h"
#include "app_common/common_func.h"
#include "app_common/ObjectPallocator.h"
#include "common/rapidjson/document.h"
#include "common/rapidjson/writer.h"
#include "common/rapidjson/stringbuffer.h"
#include "proto/common_data.pb.h"
#include "proto/ugc.pb.h"
#include "common/geohash.h"
#include "geo_func.h"
#include <string>

UgcPostProcessor::UgcPostProcessor()
{
}

UgcPostProcessor::~UgcPostProcessor()
{
}

bool UgcPostProcessor::init(ngx_http_request_t* r, session_t* s)
{
    ::Cell::PostUGCReqMsg* req = ObjectPallocator< ::Cell::PostUGCReqMsg>::allocate(r->pool);
    m_Response = (void*)ObjectPallocator< ::Cell::PostUGCRespMsg>::allocate(r->pool);
    if (!req->ParseFromArray(s->message_data.data, s->message_data.len)) {
        LOG_ERROR(r->connection->log, "parse PostUgcReqMsg failed");
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

    // generate create time
    struct tm *tmp = localtime((const time_t*)&s->timestamp);
    u_char* add_time = (u_char*)ngx_pcalloc(r->pool, 32);
    setData("add_time", (void*)add_time);

    ngx_snprintf(add_time, 32, "%04D-%02D-%02D %02D:%02D:%02D", tmp->tm_year + 1900, tmp->tm_mon + 1, tmp->tm_mday, tmp->tm_hour, tmp->tm_min, tmp->tm_sec);

    const char* table = NULL;
    if (::Cell::PT_POST == req->post_type()) {
        table = "message_post";
    } else {
        table = "message_pass";
    }


    //set mysql query string
    ngx_str_t mysql_query_string;
    mysql_query_string.data = (u_char*)ngx_palloc(r->pool, 256);
    p = ngx_snprintf(mysql_query_string.data, 256, "insert into %s(msid, mid, org_uid, to_uid, latitude, longitude, create_time) values"
            "(%uL, %uL, %D, %D, %.6f, %.6f, '%s')", table, req->msid(), req->ugc_id(), req->org_uid(), req->uid(), req->geo().latitude(), req->geo().longitude(), add_time);
    mysql_query_string.len = p - mysql_query_string.data;
    set_mysql_query_string(r, mysql_query_string);

    return BaseProcessor::init(r, s);
}

void UgcPostProcessor::clear()
{
    if (m_Request) {
        ::Cell::PostUGCReqMsg* req = (::Cell::PostUGCReqMsg*)m_Request;
        ObjectPallocator< ::Cell::PostUGCReqMsg>::free(req);
    }
    if (m_Response) {
        ::Cell::PostUGCRespMsg* resp = (::Cell::PostUGCRespMsg*)m_Response;
        ObjectPallocator< ::Cell::PostUGCRespMsg>::free(resp);
    }
}

ngx_str_t UgcPostProcessor::getSessionToken() const
{
    ngx_str_t st = ngx_null_string;
    ::Cell::PostUGCReqMsg* req = (::Cell::PostUGCReqMsg*)m_Request;
    st.data = (u_char*)req->session_token().c_str();
    st.len = req->session_token().length();
    return st;

}

void UgcPostProcessor::handerInsertPostResult(ngx_str_t* result)
{
    if (result->data && result->len) {
        if (result->len == sizeof(ERROR_STRING) - 1 && 
                0 == ngx_strncmp(result->data, ERROR_STRING, result->len)) {
            LOG_ERROR(m_R->connection->log, "mysql query failed");
            setRetcode(RET_MYSQL_RESULT_INVALID);
            return ;
        }

        std::string json;
        json.assign((char*)result->data, result->len);
        rapidjson::Document dom;
        dom.Parse(json.c_str());
        if (dom.HasParseError()) {
            LOG_ERROR(m_R->connection->log, "parse mysql insert result error: %V", result);
            setRetcode(RET_MYSQL_RESULT_INVALID);
            return ;
        }

        if (!dom.HasMember("insert_id")) {
            LOG_ERROR(m_R->connection->log, "mysql insert  result error: %V", result);
            setRetcode(RET_MYSQL_RESULT_INVALID);
            return ;
        }

        uint64_t mstid = dom["insert_id"].GetUint64();
        LOG_DEBUG(m_R->connection->log, "insert mstid: %L", mstid);
        return ;
    }
    setRetcode(RET_MYSQL_RESULT_INVALID);
}

void UgcPostProcessor::handerPostAction()
{
    // do nothing now
}

void UgcPostProcessor::handerPassAction()
{
    ::Cell::PostUGCReqMsg* req = (::Cell::PostUGCReqMsg*)m_Request;

    ngx_str_t mysql_query_string;
    mysql_query_string.data = (u_char*)ngx_palloc(m_R->pool, 256);
    u_char* p = ngx_snprintf(mysql_query_string.data, 256, "insert into msg_blacklist_%D(uid, mid) values(%D, %uL)", req->uid() & BLACKLIST_TABLE_MASK, req->uid(), req->ugc_id());
    mysql_query_string.len = p - mysql_query_string.data;
    set_mysql_query_string(m_R, mysql_query_string);
}

void UgcPostProcessor::handerUgcSelectAchieveType(ngx_str_t* type)
{
    ::Cell::PostUGCReqMsg* req = (::Cell::PostUGCReqMsg*)m_Request;
    ngx_str_t mysql_query_string;
    u_char* p = NULL;
    mysql_query_string.data = (u_char*)ngx_palloc(m_R->pool, 256);
    if (type->len == sizeof(UGC_ACHIEVE_TYPE_MSG) - 1 && 
            0 == ngx_strncmp(type->data, UGC_ACHIEVE_TYPE_MSG, type->len)) {
        setData("ugc_select_achieve_type", (void*)UGC_ACHIEVE_TYPE_MSG);
        p = ngx_snprintf(mysql_query_string.data, 256, "select post_num, pass_num, alive, max_depth, max_dist, total_dist "
                "from message_achievement where mid = %uL limit 1", req->ugc_id());
        
    } else {
        setData("ugc_select_achieve_type", (void*)UGC_ACHIEVE_TYPE_SEED);
        p = ngx_snprintf(mysql_query_string.data, 256, "select alive, depth, geohash, total_dist from message_seed where "
                "msid = %uL limit 1", req->msid());
    }
    mysql_query_string.len = p - mysql_query_string.data;
    set_mysql_query_string(m_R, mysql_query_string);
}

void UgcPostProcessor::handerUgcSelectAcieveResult(ngx_str_t* result)
{
    if (result->data && result->len) {
        if (result->len == sizeof(ERROR_STRING) - 1 && 
                0 == ngx_strncmp(result->data, ERROR_STRING, result->len)) {
            LOG_ERROR(m_R->connection->log, "query mysql failed");
            setRetcode(RET_MYSQL_RESULT_INVALID);
            return ;
        }
        std::string json;
        json.assign((char*)result->data, result->len);
        rapidjson::Document dom;
        dom.Parse(json.c_str());
        if (dom.HasParseError() || !dom.IsArray() || dom.Size() == 0) {
            LOG_ERROR(m_R->connection->log, "parse json failed: %V", result);
            setRetcode(RET_MYSQL_RESULT_INVALID);
            return ;
        }

        rapidjson::Value& value = dom[0];
        u_char* achieve_type = (u_char*)getData("ugc_select_achieve_type");
        if (0 == ngx_strncmp(achieve_type, UGC_ACHIEVE_TYPE_MSG, sizeof(UGC_ACHIEVE_TYPE_MSG) - 1)) {
            m_UgcMsgAchieve.post_num = value["post_num"].GetUint();
            m_UgcMsgAchieve.pass_num = value["pass_num"].GetUint();
            m_UgcMsgAchieve.max_depth = value["max_depth"].GetUint();
            m_UgcMsgAchieve.max_dist = value["max_dist"].GetDouble();
            m_UgcMsgAchieve.total_dist = value["total_dist"].GetDouble();
        } else {
            m_UgcSeedAchieve.alive = value["alive"].GetUint();
            m_UgcSeedAchieve.depth = value["depth"].GetUint();
            m_UgcSeedAchieve.total_dist = value["total_dist"].GetDouble();
            const char* geohash = value["geohash"].GetString();
            setData("seed_geohash", (void*)geohash);
            GeoCoord coord = geohash_decode((char*)geohash);
            m_UgcSeedAchieve.latitude = coord.latitude;
            m_UgcSeedAchieve.longitude = coord.longitude;
        }
        return ;
    }
    setRetcode(RET_MYSQL_RESULT_INVALID);
}

inline static double max(double a, double b)
{
    if (a >= b) {
        return a;
    }
    return b;
}

void UgcPostProcessor::handerUgcUpdateAchieveType(ngx_str_t* type)
{
    ::Cell::PostUGCReqMsg* req = (::Cell::PostUGCReqMsg*)m_Request;
    ngx_str_t mysql_query_string;
    u_char* p = NULL;
    u_char* add_time = (u_char*)getData("add_time");
    mysql_query_string.data = (u_char*)ngx_palloc(m_R->pool, 512);

    double dist = calculateDistSim1(req->geo().latitude(), req->geo().longitude(), 
                    m_UgcSeedAchieve.latitude, m_UgcSeedAchieve.longitude);
    
    if (type->len == sizeof(UGC_ACHIEVE_TYPE_MSG) - 1 && 
            0 == ngx_strncmp(type->data, UGC_ACHIEVE_TYPE_MSG, type->len)) {
        uint32_t max_depth = max(m_UgcMsgAchieve.max_depth, m_UgcSeedAchieve.depth + 1);
        double max_dist = max(m_UgcMsgAchieve.max_dist, m_UgcSeedAchieve.total_dist + dist);
        double total_dist = m_UgcMsgAchieve.total_dist + dist;

        if (::Cell::PT_POST == req->post_type()) {
            m_UgcMsgAchieve.max_depth = max_depth;
            m_UgcMsgAchieve.max_dist = max_dist;
            m_UgcMsgAchieve.total_dist = total_dist;
            m_UgcMsgAchieve.alive += 5;
            m_UgcMsgAchieve.post_num += 1;
            p = ngx_snprintf(mysql_query_string.data, 512, "update message_achievement set post_num = post_num %%2B 1, alive = alive %%2B 5, max_depth = %D, "
                    "max_dist = %.3f, total_dist = %.3f, update_time = '%s' where mid = %uL", max_depth, max_dist, total_dist, add_time, req->ugc_id());
        } else {
            m_UgcMsgAchieve.alive -= 1;
            m_UgcMsgAchieve.pass_num += 1;
            p = ngx_snprintf(mysql_query_string.data, 512, "update message_achievement set pass_num = pass_num %%2B 1, alive = alive - 1, "
                    "update_time = '%s' where mid = %uL", add_time, req->ugc_id());
        }
    } else {
        if (::Cell::PT_POST == req->post_type()) {
            m_UgcSeedAchieve.dist = dist;
            m_UgcSeedAchieve.total_dist += dist;
            m_UgcSeedAchieve.alive += 5;
            p = ngx_snprintf(mysql_query_string.data, 512, "update message_seed set alive = alive %%2B 5, depth = depth %%2B 1, dist = %.3f, "
                    "total_dist = total_dist %%2B %.3f, update_time = '%s' where msid = %uL", dist, dist, add_time, req->msid());
        } else {
            m_UgcSeedAchieve.alive -= 1;
            p = ngx_snprintf(mysql_query_string.data, 512, "update message_seed set alive = alive - 1, update_time = '%s' where msid = %uL", 
                    dist, dist, add_time, req->msid());
        }
    }
    mysql_query_string.len = p - mysql_query_string.data;
    set_mysql_query_string(m_R, mysql_query_string);
}

void UgcPostProcessor::handerUgcUpdateAchieveResult(ngx_str_t* result)
{
    if (result->data && result->len) {
        if (result->len == sizeof(ERROR_STRING) - 1 && 
                0 == ngx_strncmp(result->data, ERROR_STRING, result->len)) {
            LOG_WARN(m_R->connection->log, "query mysql failed in handerUgcUpdateAchieveResult");
            return ;
        }
        std::string json;
        json.assign((char*)result->data, result->len);
        rapidjson::Document dom;
        dom.Parse(json.c_str());
        if (dom.HasParseError() || !dom.IsObject()) {
            LOG_WARN(m_R->connection->log, "parse json failed: %V in handerUgcUpdateAchieveResult", result);
            return ;
        }

        if (!dom.HasMember("errcode") || dom["errcode"].GetUint() != 0) {
            LOG_WARN(m_R->connection->log, "update ugc achieve failed: %V in handerUgcUpdateAchieveResult", result);
            return ;
        }
        LOG_DEBUG(m_R->connection->log, "handerUgcUpdateAchieveResult: %V", result);
    }
}

void UgcPostProcessor::handerLocalSeed()
{
    ::Cell::PostUGCReqMsg* req = (::Cell::PostUGCReqMsg*)m_Request;
    ::Cell::Geo geo = req->geo();
    char *geo_hash = geohash_encode(m_R->pool, geo.latitude(), geo.longitude(), 5);
    char *seed_geohash = (char*)getData("seed_geohash");
    if (::Cell::PT_PASS == req->post_type() || geo_hash || 0 == ngx_strncmp(geo_hash, seed_geohash, 5)) {
        setData("local_seed", (void*)OK_STRING);
    } else {
        char* add_time = (char*)getData("add_time");
        ngx_str_t mysql_query_string;
        mysql_query_string.data =(u_char*)ngx_palloc(m_R->pool, 256);
        u_char* p = ngx_snprintf(mysql_query_string.data, 256, "insert into message_seed(uid, mid, geohash, update_time, create_time, from_msid)"
                " values(%D, %uL, '%s', '%s', '%s', %uL)", req->org_uid(), req->ugc_id(), geo_hash, add_time, add_time, req->msid());
        mysql_query_string.len = p - mysql_query_string.data;
        set_mysql_query_string(m_R, mysql_query_string);
    }
}

void UgcPostProcessor::handerUserAchieveType(ngx_str_t* type)
{
    ::Cell::PostUGCReqMsg* req = (::Cell::PostUGCReqMsg*)m_Request;
    ngx_str_t redis_query_string;
    u_char* p = NULL;
    redis_query_string.data = (u_char*)ngx_palloc(m_R->pool, 32);

    if (type->len == sizeof(USER_ACHIEVE_TYPE_TO) - 1 && 
            0 == ngx_strncmp(type->data, USER_ACHIEVE_TYPE_TO, type->len)) {
        setData("user_achieve_type", (void*)USER_ACHIEVE_TYPE_TO);
        p = ngx_snprintf(redis_query_string.data, 32, "get %D_a", req->uid());
    } else {
        setData("user_achieve_type", (void*)USER_ACHIEVE_TYPE_FROM);
        p = ngx_snprintf(redis_query_string.data, 32, "get %D_a", req->org_uid());
    }
    redis_query_string.len = p - redis_query_string.data;
    set_redis_query_string(m_R, redis_query_string);
}

void UgcPostProcessor::handerUserAchieveResult(ngx_str_t* result)
{
    ::Cell::PostUGCReqMsg* req = (::Cell::PostUGCReqMsg*)m_Request;
    ngx_str_t mysql_query_string;
    u_char* p = NULL;
    u_char* achieve_type = (u_char*)getData("user_achieve_type");
    mysql_query_string.data = (u_char*)ngx_palloc(m_R->pool, 512);

    if (result->data && result->len) {
        setRetcode(RET_SUCCESS);
        if (result->len == sizeof(ERROR_STRING) - 1 && 
                0 == ngx_strncmp(result->data, ERROR_STRING, result->len)) {
            LOG_DEBUG(m_R->connection->log, "get redis failed in handerUserAchieveResult for type: %s", achieve_type);
            setRetcode(RET_REDIS_RESULT_INVALID);
            goto no_redis;
        }
        if (result->len == sizeof(REDIS_NIL_STRING) - 1 && 
                0 == ngx_strncmp(result->data, REDIS_NIL_STRING, result->len)) {
            LOG_DEBUG(m_R->connection->log, "redis is nil");
            setRetcode(RET_REDIS_RESULT_INVALID);
            goto no_redis;
        }

        std::string json;
        json.assign((char*)result->data, result->len);
        rapidjson::Document dom;
        dom.Parse(json.c_str());
        if (dom.HasParseError()) {
            LOG_WARN(m_R->connection->log, "parse achieve result json error: %V", result);
            setRetcode(RET_REDIS_RESULT_INVALID);
            goto no_redis;
        }

        if (!dom.IsArray() || dom.Size() != 1) {
            LOG_WARN(m_R->connection->log, "achieve json result is invalid: %V", result);
            setRetcode(RET_REDIS_RESULT_INVALID);
            goto no_redis;
        }
        
        rapidjson::Value& value = dom[0];
        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        ngx_str_t redis_query_string;
        redis_query_string.data = (u_char*)ngx_palloc(m_R->pool, 512);

        if (0 == ngx_strncmp(achieve_type, USER_ACHIEVE_TYPE_TO, sizeof(USER_ACHIEVE_TYPE_TO) - 1)) {
            if (::Cell::PT_POST == req->post_type()) {
                value["total_post_num"].SetUint(value["total_post_num"].GetUint() + 1);
            } else {
                value["total_pass_num"].SetUint(value["total_pass_num"].GetUint() + 1);
            }
            dom.Accept(writer);
            p = ngx_snprintf(redis_query_string.data, 512, "set %D_a '%s' EX 1800", req->uid(), buffer.GetString());
        } else {
            if (::Cell::PT_POST == req->post_type()) {
                value["total_posted_num"].SetUint(value["total_posted_num"].GetUint() + 1);
                value["total_alive"].SetUint(value["total_alive"].GetUint() + 5);
                value["total_dist"].SetDouble(value["total_dist"].GetDouble() + m_UgcSeedAchieve.dist);
                value["max_dist"].SetDouble(max(value["max_dist"].GetDouble(), m_UgcMsgAchieve.max_dist));
            } else {
                value["total_passed_num"].SetUint(value["total_passed_num"].GetUint() + 1);
                value["total_alive"].SetUint(value["total_alive"].GetUint() - 1);
            }
            dom.Accept(writer);
            p = ngx_snprintf(redis_query_string.data, 512, "set %D_a '%s' EX 1800", req->org_uid(), buffer.GetString());
        }
        redis_query_string.len = p - redis_query_string.data;
        set_redis_query_string(m_R, redis_query_string);
    }
    // set update user achievement mysql query
no_redis:
    if (0 == ngx_strncmp(achieve_type, USER_ACHIEVE_TYPE_TO, sizeof(USER_ACHIEVE_TYPE_TO) - 1)) {
        if (::Cell::PT_POST == req->post_type()) {
            p = ngx_snprintf(mysql_query_string.data, 512, "update user_achievement set total_post_num = total_post_num %%2B 1 where uid = %D", req->uid());
        } else {
            p = ngx_snprintf(mysql_query_string.data, 512, "update user_achievement set total_pass_num = total_pass_num %%2B 1 where uid = %D", req->uid());
        }
    } else {
        if (::Cell::PT_POST == req->post_type()) {
            p = ngx_snprintf(mysql_query_string.data, 512, "update user_achievement set total_posted_num = total_posted_num %%2B 1, "
                    "total_alive = total_alive %%2B 5, total_dist = total_dist %%2B %.3f, max_dist = (case when max_dist >= %.3f then max_dist "
                    "else %.3f end) where uid = %D", m_UgcSeedAchieve.dist, m_UgcMsgAchieve.max_dist, m_UgcMsgAchieve.max_dist, req->org_uid());
        } else {
            p = ngx_snprintf(mysql_query_string.data, 512, "update user_achievement set total_passed_num = total_passed_num %%2B 1, total_alive = total_alive - 1 where uid = %D", req->org_uid());
        }
    }
    mysql_query_string.len = p - mysql_query_string.data;
    set_mysql_query_string(m_R, mysql_query_string);
}

void UgcPostProcessor::buildResponse()
{
    ::Cell::PostUGCRespMsg* resp = (::Cell::PostUGCRespMsg*)m_Response;
    switch (m_Retcode) {
        case RET_REDIS_RESULT_INVALID:
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
