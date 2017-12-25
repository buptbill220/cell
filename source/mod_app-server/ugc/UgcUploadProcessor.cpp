#include "ugc/UgcUploadProcessor.h"
#include "app_common/ObjectPallocator.h"
#include "app_common/mysql_common.h"
#include "app_common/app_log.h"
#include "app_common/common_func.h"
#include "proto/common_data.pb.h"
#include "proto/ugc.pb.h"
#include "common/geohash.h"
#include "common/rapidjson/writer.h"
#include "common/rapidjson/stringbuffer.h"
#include "common/rapidjson/document.h"
#include <stdio.h>

UgcUploadProcessor::UgcUploadProcessor() : ugc_id(0)
{
}

UgcUploadProcessor::~UgcUploadProcessor()
{
}

bool UgcUploadProcessor::init(ngx_http_request_t* r, session_t* s)
{
    ::Cell::UploadUGCReqMsg* req = ObjectPallocator< ::Cell::UploadUGCReqMsg>::allocate(r->pool);
    m_Response = (void*)ObjectPallocator< ::Cell::UploadUGCRespMsg>::allocate(r->pool);
    if (!req) {
        LOG_ERROR(r->connection->log, "palloc UploadUGCReqMsg faild");
        setRetcode(RET_INTERNAL_ERROR);
        return false;
    }
    if (!req->ParseFromArray(s->message_data.data, s->message_data.len)) {
        LOG_ERROR(r->connection->log, "parse UploadUGCReqMsg failed");
        setRetcode(RET_INVALID_REQ);
        return false;
    }
    m_Request = (const void*)req;
    
    // set check user session query
    ngx_str_t redis_query_string;
    redis_query_string.data = (u_char*)ngx_palloc(r->pool, 128);
    u_char* p = ngx_snprintf(redis_query_string.data, 128, "get %d", req->uid());
    redis_query_string.len = p - redis_query_string.data;
    set_redis_query_string(r, redis_query_string);

    // set insert messsage sql query
    ngx_str_t mysql_query_string;
    mysql_query_string.data = (u_char*)ngx_palloc(r->pool, 4096);
    const ::Cell::UGC& ugc = req->ugc();
    const ::Cell::Geo& geo = ugc.geo();
    const ::Cell::MessageMeta& meta_msg = ugc.message_meta();

    // generate geohash
    char *geo_hash = geohash_encode(r->pool, geo.latitude(), geo.longitude(), 5);
    if (NULL == geo_hash) {
        LOG_ERROR(r->connection->log, "geo is invalid,<%.6f,%.6f>", geo.latitude(), geo.longitude());
        setRetcode(RET_INVALID_REQ);
        return false;
    }
    setData("geo_hash", (void*)geo_hash);

    // generate create time
    struct tm *tmp = localtime((const time_t*)&s->timestamp);
    u_char* add_time = (u_char*)ngx_pcalloc(r->pool, 32);
    setData("add_time", (void*)add_time);

    ngx_snprintf(add_time, 32, "%04D-%02D-%02D %02D:%02D:%02D", tmp->tm_year + 1900, tmp->tm_mon + 1, tmp->tm_mday, tmp->tm_hour, tmp->tm_min, tmp->tm_sec);

    // escape ugc content in case of special charater
    ngx_str_t escape_content;
    escape_content.data = (u_char*)ngx_palloc(r->pool, meta_msg.content().length() * 2);
    escape_content.len = mysql_escape_string((char*)escape_content.data, 
            meta_msg.content().c_str(), meta_msg.content().length());

    // format query string
    std::string image, size;
    if (meta_msg.image_size()) {
        image = meta_msg.image(0);
        size = meta_msg.img_size(0);
    }
    p = ngx_snprintf(mysql_query_string.data, 4096, "insert into message(uid, content, post_images, images_size, "
            "geohash, latitude, longitude, card_type, create_time) values(%D, '%V', '%s', '%s', '%s', %3.6f, %3.6f,"
            "'%d', '%s')", req->uid(), &escape_content, image.c_str(), size.c_str(), geo_hash, geo.latitude(), 
            geo.longitude(), ugc.card_type(), add_time);
    mysql_query_string.len = p - mysql_query_string.data;
    set_mysql_query_string(r, mysql_query_string);

    return BaseProcessor::init(r, s);
}

void UgcUploadProcessor::clear()
{
    if (m_Request) {
        ::Cell::UploadUGCReqMsg* req = (::Cell::UploadUGCReqMsg*)m_Request;
        ObjectPallocator< ::Cell::UploadUGCReqMsg>::free(req);
    }
    if (m_Response) {
        ::Cell::UploadUGCRespMsg* res = (::Cell::UploadUGCRespMsg*)m_Response;
        ObjectPallocator< ::Cell::UploadUGCRespMsg>::free(res);
    }
}

void UgcUploadProcessor::handerInsertUgcResult(ngx_str_t* result)
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
            LOG_ERROR(m_R->connection->log, "parse mysql insert result error: %V", result);
            setRetcode(RET_MYSQL_RESULT_INVALID);
            return ;
        }

        if (!dom.HasMember("insert_id")) {
            LOG_ERROR(m_R->connection->log, "mysql insert result error: %V", result);
            setRetcode(RET_MYSQL_RESULT_INVALID);
            return ;
        }

        ugc_id = dom["insert_id"].GetUint64();
        // set insert message achieve sql query
        ngx_str_t mysql_query_string;
        mysql_query_string.data = (u_char*)ngx_palloc(m_R->pool, 256);
        u_char* add_time = (u_char*)getData("add_time");
        u_char* p = ngx_snprintf(mysql_query_string.data, 256, "insert into message_achievement(mid, create_time, update_time) values"
                "(%uL, '%s', '%s')", ugc_id, add_time, add_time);
        mysql_query_string.len = p - mysql_query_string.data;
        set_mysql_query_string(m_R, mysql_query_string);

        return ;
    }
    setRetcode(RET_MYSQL_RESULT_INVALID);
}

void UgcUploadProcessor::handerInsertAchieveResult(ngx_str_t* result)
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
            LOG_ERROR(m_R->connection->log, "parse mysql insert result error: %V", result);
            setRetcode(RET_MYSQL_RESULT_INVALID);
            return ;
        }

        if (!dom.HasMember("insert_id")) {
            LOG_ERROR(m_R->connection->log, "mysql insert result error: %V", result);
            setRetcode(RET_MYSQL_RESULT_INVALID);
            return ;
        }
        uint64_t ugc_aid = dom["insert_id"].GetUint64();
        LOG_DEBUG(m_R->connection->log, "insert maid: %L to message_achievement successfully", ugc_aid);

        u_char* add_time = (u_char*)getData("add_time");
        u_char* geo_hash = (u_char*)getData("geo_hash");

        ::Cell::UploadUGCReqMsg* req = (::Cell::UploadUGCReqMsg*)m_Request;

        ngx_str_t mysql_query_string;
        mysql_query_string.data = (u_char*)ngx_palloc(m_R->pool, 256);
        u_char* p = ngx_snprintf(mysql_query_string.data, 256, "insert into message_seed(uid, mid, geohash, update_time, create_time)"
                " values(%D, %uL, '%s', '%s', '%s')", req->uid(), ugc_id, geo_hash, add_time, add_time);
        mysql_query_string.len = p - mysql_query_string.data;
        set_mysql_query_string(m_R, mysql_query_string);

        return ;
    }
    setRetcode(RET_MYSQL_RESULT_INVALID);
}

void UgcUploadProcessor::handerInsertSeedResult(ngx_str_t* result)
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
            LOG_ERROR(m_R->connection->log, "parse mysql insert result error: %V", result);
            setRetcode(RET_MYSQL_RESULT_INVALID);
            return ;
        }

        if (!dom.HasMember("insert_id")) {
            LOG_ERROR(m_R->connection->log, "mysql insert result error: %V", result);
            setRetcode(RET_MYSQL_RESULT_INVALID);
            return ;
        }

        uint64_t ugc_cid = dom["insert_id"].GetUint64();
        LOG_DEBUG(m_R->connection->log, "insert messge_seed id: %uL successfully", ugc_cid);

        ::Cell::UploadUGCReqMsg* req = (::Cell::UploadUGCReqMsg*)m_Request;
        ngx_str_t redis_query_string;
        redis_query_string.data = (u_char*)ngx_palloc(m_R->pool, 32);
        u_char* p = ngx_snprintf(redis_query_string.data, 32, "get %D_a", req->uid());
        redis_query_string.len = p - redis_query_string.data;
        set_redis_query_string(m_R, redis_query_string);

        return ;
    }
    setRetcode(RET_MYSQL_RESULT_INVALID);
}

void UgcUploadProcessor::handerUserAchieveResult(ngx_str_t* result)
{
    ::Cell::UploadUGCReqMsg* req = (::Cell::UploadUGCReqMsg*)m_Request;
    const char* add_time = (const char*)getData("add_time");
    u_char* p = NULL;

    if (result->data && result->len) {
        std::string json;
        rapidjson::Document dom;
        rapidjson::StringBuffer buffer;
        ngx_str_t redis_query_string;

        if (result->len == sizeof(ERROR_STRING) - 1 && 
                0 == ngx_strncmp(result->data, ERROR_STRING, result->len)) {
            LOG_DEBUG(m_R->connection->log, "get user achievement from redis failed");
            goto no_redis;
        }

        json.assign((char*)result->data, result->len);
        dom.Parse(json.c_str());

        if (dom.HasParseError()) {
            LOG_DEBUG(m_R->connection->log, "parse redis user achievement failed: %V", result);
            goto no_redis;
        }

        if (!dom.IsArray() || dom.Size() != 1) {
            LOG_DEBUG(m_R->connection->log, "redis user achievement value invalid: %V", result);
            goto no_redis;
        }

        rapidjson::Value& value = dom[0];
        if (!value.HasMember("total_create")) {
            LOG_DEBUG(m_R->connection->log, "redis user achievement value invalid: %V", result);
            goto no_redis;
        }

        value["total_create"].SetUint(value["total_create"].GetUint() + 1);
        value["total_alive"].SetUint(value["total_alive"].GetUint() + 5);
        value["update_time"].SetString("test");

        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        dom.Accept(writer);

        // update redis
        redis_query_string.data = (u_char*)ngx_palloc(m_R->pool, 512);
        p = ngx_snprintf(redis_query_string.data, 512, "set %D_a '%s' EX 1800", req->uid(), buffer.GetString());
        redis_query_string.len = p - redis_query_string.data;
        set_redis_query_string(m_R, redis_query_string);

        setData("redis_status", (void*)OK_STRING);
    }

no_redis:
    ngx_str_t mysql_query_string;
    mysql_query_string.data = (u_char*)ngx_palloc(m_R->pool, 256);
    // 需要对+号进行urlencode，否则会被转换成空格，还需要注意ngx格式处理
    p = ngx_snprintf(mysql_query_string.data, 256, "update user_achievement set total_create = total_create %%2B 1, "
            "total_alive = total_alive %%2B 5, update_time = '%s' where uid = %D", add_time, req->uid());
    mysql_query_string.len = p - mysql_query_string.data;
    set_mysql_query_string(m_R, mysql_query_string);
}

ngx_str_t UgcUploadProcessor::getSessionToken() const
{
    ngx_str_t st = ngx_null_string;
    ::Cell::UploadUGCReqMsg* req = (::Cell::UploadUGCReqMsg*)m_Request;
    st.data = (u_char*)req->session_token().c_str();
    st.len = req->session_token().length();
    return st;
}

void UgcUploadProcessor::buildResponse()
{
    ::Cell::UploadUGCRespMsg* resp = (::Cell::UploadUGCRespMsg*)m_Response;
    switch (m_Retcode) {
        case RET_SUCCESS:
            resp->set_ret_code(::Cell::RC_SUCCESS);
            resp->set_ugc_id(ugc_id);
            break;
        case RET_SESSION_INVALID:
            resp->set_ret_code(::Cell::RC_SESSION_INVALID);
            break;
        default:
            resp->set_ret_code(::Cell::RC_FAILED);
    }
}
