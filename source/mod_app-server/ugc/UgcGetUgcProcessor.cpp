#include "ugc/UgcGetUgcProcessor.h"
#include "app_common/app_log.h"
#include "app_common/common_func.h"
#include "app_common/ObjectPallocator.h"
#include "app_common/geo_func.h"
#include "common/rapidjson/document.h"
#include "proto/common_data.pb.h"
#include "proto/ugc.pb.h"
#include <string>
#include <tr1/unordered_map>

UgcGetUgcProcessor::UgcGetUgcProcessor()
{
}

UgcGetUgcProcessor::~UgcGetUgcProcessor()
{
}

bool UgcGetUgcProcessor::init(ngx_http_request_t* r, session_t* s)
{
    ::Cell::GetUGCReqMsg* req = ObjectPallocator< ::Cell::GetUGCReqMsg>::allocate(r->pool);
    m_Response = (void*)ObjectPallocator< ::Cell::GetUGCRespMsg>::allocate(r->pool);
    if (!req->ParseFromArray(s->message_data.data, s->message_data.len)) {
        LOG_ERROR(r->connection->log, "parse GetUGCReqMsg failed");
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
    mysql_query_string.data = (u_char*)ngx_palloc(r->pool, 1024);
    u_char mids[1024] = "(";
    u_char* p_mids = mids + 1;
    //最多一次取5个
    for (int i = 0; i < 5 && i < req->ugc_id_size(); ++i) {
        p_mids = ngx_sprintf(p_mids, "%L,", req->ugc_id(i));
    }
    *(p_mids-1) = ')';
    *p_mids = 0;
    p = ngx_snprintf(mysql_query_string.data, 1024, "select message.mid as mid, message.uid as uid, content, gender, "
            "post_images, card_type, images_size, geohash, message.latitude as latitude, message.longitude as "
            "longitude, user.latitude as latitude1, user.longitude as longitude1, post_num, pass_num, total_dist, alive, "
            "avada_url, user_name from message inner join message_achievement on message.mid = message_achievement.mid "
            "inner join user on message.uid = user.uid where message.mid in %s", mids);
    mysql_query_string.len = p - mysql_query_string.data;
    set_mysql_query_string(r, mysql_query_string);
    LOG_DEBUG(r->connection->log, "set mysql_query_string: %V", &mysql_query_string);

    return BaseProcessor::init(r, s);
}

void UgcGetUgcProcessor::clear()
{
    if (m_Request) {
        ::Cell::GetUGCReqMsg* req = (::Cell::GetUGCReqMsg*)m_Request;
        ObjectPallocator< ::Cell::GetUGCReqMsg>::free(req);
    }
    if (m_Response) {
        ::Cell::GetUGCRespMsg* resp = (::Cell::GetUGCRespMsg*)m_Response;
        ObjectPallocator< ::Cell::GetUGCRespMsg>::free(resp);
    }
}

ngx_str_t UgcGetUgcProcessor::getSessionToken() const
{
    ngx_str_t st = ngx_null_string;
    ::Cell::GetUGCReqMsg* req = (::Cell::GetUGCReqMsg*)m_Request;
    st.data = (u_char*)req->session_token().c_str();
    st.len = req->session_token().length();
    return st;
}

void UgcGetUgcProcessor::handerMysqlQueryResult(ngx_str_t* result)
{
    if (result->data && result->len) {
        if (result->len == sizeof(ERROR_STRING) - 1 && 
                0 == ngx_strncmp(result->data, ERROR_STRING, result->len)) {
            LOG_ERROR(m_R->connection->log, "mysql query error");
            setRetcode(RET_MYSQL_RESULT_INVALID);
            return ;
        }
        std::string json;
        json.assign((char*)result->data, result->len);
        rapidjson::Document dom;
        dom.Parse(json.c_str());
        if (dom.HasParseError()) {
            LOG_ERROR(m_R->connection->log, "parse mysql query result json failed");
            setRetcode(RET_MYSQL_RESULT_INVALID);
            return ;
        }
        if (!dom.IsArray() || 0 == dom.Size()) {
            LOG_ERROR(m_R->connection->log, "json result is not array or null: %v", result);
            setRetcode(RET_MYSQL_RESULT_INVALID);
            return ;
        }
        ::Cell::GetUGCReqMsg* req = (::Cell::GetUGCReqMsg*)m_Request;
        ::Cell::Geo req_geo = req->geo();

        unordered_map<uint64_t, uint32_t> UgcMap;
        for (size_t i = 0; i < dom.Size(); ++i) {
            rapidjson::Value& value = dom[i];
            uint64_t mid = value["mid"].GetUint64();
            UgcMap[mid] = i;
        }

        ::Cell::GetUGCRespMsg* resp = (::Cell::GetUGCRespMsg*)m_Response;
        unordered_map<uint64_t, uint32_t>::iterator it;
        for (int i = 0; i < 5 && i < req->ugc_id_size(); ++i) {
            it = UgcMap.find(req->ugc_id(i));
            if (it == UgcMap.end()) {
                LOG_ERROR(m_R->connection->log, "cannt find ugc detail for msg id: %L", req->ugc_id(i));
                setRetcode(RET_FAILED);
                return ;
            }
            rapidjson::Value& value = dom[it->second];
            ::Cell::UGC* ugc = resp->add_ugc();
            ::Cell::MessageMeta* meta = ugc->mutable_message_meta();
            ugc->set_card_type((::Cell::CardType)value["card_type"].GetUint());
            meta->set_content(value["content"].GetString());
            meta->add_image(value["post_images"].GetString());
            meta->add_img_size(value["images_size"].GetString());
        
            ::Cell::Geo* geo = ugc->mutable_geo();
            geo->set_latitude(value["latitude"].GetDouble());
            geo->set_longitude(value["longitude"].GetDouble());

            resp->add_post_num(value["post_num"].GetUint());
            resp->add_pass_num(value["pass_num"].GetUint());
            resp->add_post_dist((uint32_t)value["total_dist"].GetDouble());
            resp->add_usr_name(value["user_name"].GetString());
            resp->add_avada_url(value["avada_url"].GetString());
            resp->add_dist((uint32_t)calculateDistSim1(req_geo.latitude(), req_geo.longitude(), 
                        value["latitude1"].GetDouble(), value["longitude1"].GetDouble()));
            resp->add_gender(::Cell::Gender(value["gender"].GetUint()));
            resp->add_alive(value["alive"].GetUint());
            resp->add_card_type(::Cell::CardType(value["card_type"].GetUint()));
        }

        return ;
    }
    setRetcode(RET_MYSQL_RESULT_INVALID);
}

void UgcGetUgcProcessor::buildResponse()
{
    ::Cell::GetUGCRespMsg* resp = (::Cell::GetUGCRespMsg*)m_Response;
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
