#include "user/user_get_achieve.h"
#include "user/UserGetAchieveProcessor.h"
#include "app_common/common_func.h"
#include "app_common/app_log.h"
#include "proto/account.pb.h"
#include <string>

ngx_int_t get_cache_achievement(ngx_http_request_t* r, ngx_str_t* data)
{
    session_t* s = get_session_ptr(r);
    UserGetAchieveProcessor* processor = (UserGetAchieveProcessor*)s->processor;
    processor->handerCacheAchieve();

    data->data = (u_char*)OK_STRING;
    data->len = sizeof(OK_STRING) - 1;
    return NGX_OK;
}

ngx_int_t set_get_user_achieve_result_r(ngx_http_request_t* r, ngx_str_t* data)
{
    session_t* s = get_session_ptr(r);
    UserGetAchieveProcessor* processor = (UserGetAchieveProcessor*)s->processor;
    processor->handerAchieveResult(data);
    if (RET_SUCCESS == processor->getRetcode()) {
        processor->setData("redis_status", (void*)OK_STRING);
    } else {
        processor->setData("redis_status", (void*)ERROR_STRING);
    }
    return NGX_OK;
}

ngx_int_t get_get_user_achieve_result_status_r(ngx_http_request_t* r, ngx_str_t* data)
{
    session_t* s = get_session_ptr(r);
    UserGetAchieveProcessor* processor = (UserGetAchieveProcessor*)s->processor;
    u_char* redis_status = (u_char*)processor->getData("redis_status");
    if (redis_status && 0 == ngx_strncmp(redis_status, OK_STRING, sizeof(OK_STRING) - 1)) {
        data->data = (u_char*)OK_STRING;
        data->len = sizeof(OK_STRING) - 1;
    } else {
        data->data = (u_char*)ERROR_STRING;
        data->len = sizeof(ERROR_STRING) - 1;
        ::Cell::GetUserProfileReqMsg* req = (::Cell::GetUserProfileReqMsg*)processor->getRequest();
        ngx_str_t mysql_query_string;
        mysql_query_string.data = (u_char*)ngx_palloc(r->pool, 512);
        u_char* p = ngx_snprintf(mysql_query_string.data, 512, "select uaid, total_create, total_post_num, total_pass_num, total_posted_num, " \
                "total_passed_num, total_alive, max_dist, total_dist, update_time, level, seed, pk_coin, pk_win_num, pk_lose_num, pk_rank " \
                "from user_achievement where uid = %d", req->org_uid());
        mysql_query_string.len = p - mysql_query_string.data;
        set_mysql_query_string(r, mysql_query_string);
    }
    return NGX_OK;
}

ngx_int_t get_get_user_achieve_result_m(ngx_http_request_t* r, ngx_str_t* data)
{
    LOG_DEBUG(r->connection->log, "get user achieve result mysql: %V", data);
    session_t* s = get_session_ptr(r);
    UserGetAchieveProcessor* processor = (UserGetAchieveProcessor*)s->processor;
    processor->handerAchieveResult(data);
    ::Cell::GetUserProfileReqMsg* req = (::Cell::GetUserProfileReqMsg*)processor->getRequest();
    if (RET_SUCCESS == processor->getRetcode()) {
        ngx_str_t redis_query_string;
        redis_query_string.data = (u_char*)ngx_palloc(r->pool, 512);
        u_char* p = ngx_snprintf(redis_query_string.data, 512, "set %D_a '%V' EX 1800", req->org_uid(), data);
        redis_query_string.len = p - redis_query_string.data;
        set_redis_query_string(r, redis_query_string);
    }
    return NGX_OK;
}

ngx_int_t get_get_user_achieve_resp(ngx_http_request_t* r, ngx_str_t* data)
{
    session_t* s = get_session_ptr(r);
    UserGetAchieveProcessor* processor = (UserGetAchieveProcessor*)s->processor;
    processor->buildResponse();
    ::Cell::GetUserAchieveRespMsg* resp = (::Cell::GetUserAchieveRespMsg*)processor->getResponse();
    std::string str_resp;
    resp->SerializeToString(&str_resp);
    data->len = str_resp.length();
    data->data = (u_char*)ngx_palloc(r->pool, data->len);
    ngx_memcpy(data->data, str_resp.c_str(), data->len);
 
    return NGX_OK;
}
