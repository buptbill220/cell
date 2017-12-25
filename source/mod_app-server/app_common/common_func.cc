#include "app_common/common_func.h"
#include "app_common/app_log.h"
#include "app_common/pv_http_head.h"
#include "app_common/ObjectPallocator.h"
#include "user/UserInitProcessor.h"
#include "user/UserRegisterProcessor.h"
#include "user/UserLoginProcessor.h"
#include "user/UserSetProfProcessor.h"
#include "user/UserGetProfProcessor.h"
#include "user/UserGetAchieveProcessor.h"
#include "user/UserGetUgcListProcessor.h"
#include "user/UserCheckRegProcessor.h"
#include "ugc/UgcUploadProcessor.h"
#include "ugc/UgcPullListProcessor.h"
#include "ugc/UgcGetUgcProcessor.h"
#include "ugc/UgcPostProcessor.h"
#include "ugc/UgcAddCommentProcessor.h"
#include "ugc/UgcGetCommentProcessor.h"
#include "ugc/UgcAddCollectionProcessor.h"
#include "ugc/UgcPlayCardProcessor.h"

#define MAX_CIRCLE_IDX 0x7fffffff

extern ngx_module_t* ngx_http_module_cell;
static ngx_http_app_main_conf_t* s_mc = NULL;
static ngx_uint_t s_cycle_index = 1;

static ngx_str_t s_message_type_values[] = {
    //bad message
    ngx_string(UNKNOWN_MESSAGE),
    //user message
    ngx_string("1"),
    ngx_string("2"),
    ngx_string("3"),
    ngx_string("4"),
    ngx_string("5"),
    ngx_string("6"),
    ngx_string("7"),
    ngx_string("8"),
    //ugc message
    ngx_string("101"),
    ngx_string("102"),
    ngx_string("103"),
    ngx_string("104"),
    ngx_string("105"),
    ngx_string("106"),
    ngx_string("107"),
    ngx_string("108")
};

enum MESSAGE_TYPE_IDX {
    UNKNOWN_MSG_IDX = 0,
    USER_INIT_MSG_IDX,
    USER_REG_MSG_IDX,
    USER_LOGIN_MSG_IDX,
    USER_SET_PROF_MSG_IDX,
    USER_GET_PROF_MSG_IDX,
    USER_GET_ACHIEVE_MSG_IDX,
    USER_GET_UGC_LIST_MSG_IDX,
    USER_CHECK_REG_MSG_IDX,
    UGC_UPLOAD_MSG_IDX,
    UGC_PULL_UGC_LIST_MSG_IDX,
    UGC_GET_UGC_MSG_IDX,
    UGC_POST_UGC_MSG_IDX,
    UGC_ADD_COM_MSG_IDX,
    UGC_GET_COM_MSG_IDX,
    UGC_ADD_CLT_MSG_IDX,
    UGC_PLAY_CARD_MSG_IDX
};

ngx_variable_value_t* get_var_ptr(ngx_http_request_t* r, ngx_int_t idx)
{
    ngx_variable_value_t* var;
    var = ngx_http_get_flushed_variable(r, idx);
    if (!var) {
        LOG_ERROR(r->connection->log, "impossible: get var error");
        return NULL;
    }
    LOG_DEBUG(r->connection->log, "idx: %d, var: %p, var.data:%p", idx, var, var->data);
    return var;
}

session_t* get_session_ptr(ngx_http_request_t* r)
{
    ngx_variable_value_t* var = get_var_ptr(r, s_mc->idx_session);
    return (session_t*)(NULL == var ? NULL : var->data);
}

ngx_int_t set_message_type(ngx_http_request_t* r, ngx_str_t data)
{
    LOG_DEBUG(r->connection->log, "set message_type: %V", &data);
    ngx_variable_value_t* var = get_var_ptr(r, s_mc->idx_message_type);
    var->data = data.data;
    var->len = data.len;
    return NGX_OK;
}

ngx_int_t set_message_data(ngx_http_request_t* r, ngx_str_t data)
{
    LOG_DEBUG(r->connection->log, "set message_data: %V", &data);
    ngx_variable_value_t* var = get_var_ptr(r, s_mc->idx_message_data);
    var->data = data.data;
    var->len = data.len;
    return NGX_OK;
}

ngx_int_t set_mysql_query_string(ngx_http_request_t* r, ngx_str_t data)
{
    LOG_DEBUG(r->connection->log, "set mysql_query_string: %V", &data);
    ngx_variable_value_t* var = get_var_ptr(r, s_mc->idx_mysql_query_string);
    var->data = data.data;
    var->len = data.len;
    return NGX_OK;
}

ngx_int_t set_redis_query_string(ngx_http_request_t* r, ngx_str_t data)
{
    LOG_DEBUG(r->connection->log, "set redis_query_string: %V", &data);
    ngx_variable_value_t* var = get_var_ptr(r, s_mc->idx_redis_query_string);
    var->data = data.data;
    var->len = data.len;
    return NGX_OK;
}

ngx_int_t set_check_session(ngx_http_request_t* r, ngx_str_t data)
{
    LOG_DEBUG(r->connection->log, "set check_session: %V", &data);
    ngx_variable_value_t* var = get_var_ptr(r, s_mc->idx_check_session);
    var->data = data.data;
    var->len = data.len;
    return NGX_OK;
}

ngx_int_t get_session_id(ngx_http_request_t* r, session_t* s)
{
    s->session_id.data = (u_char*)ngx_palloc(r->pool, 32);
    if (!s->session_id.data) {
        LOG_ERROR(r->connection->log, "palloc session id failed");
        return NGX_ERROR;
    }
    s->session_id.len = 32;
    u_char* cur = ngx_cpymem(s->session_id.data, s->main_conf->session_id_prefix.data, 16);
    ngx_snprintf(cur, 16, "%08xi%08xi", s->timestamp, s_cycle_index);
    s_cycle_index = (s_cycle_index == MAX_CIRCLE_IDX) ? 1 : s_cycle_index+1;
    return NGX_OK;
}

ngx_int_t build_session(ngx_http_request_t* r, ngx_str_t* data)
{
    s_mc = (ngx_http_app_main_conf_t*)ngx_http_get_module_main_conf(r, (*ngx_http_module_cell));
    if (!s_mc) {
        LOG_ERROR(r->connection->log, "get main_conf_t error");
        return NGX_ERROR;
    }
    
    ngx_str_t msg_type = ngx_string(UNKNOWN_MESSAGE);
    unsigned short type = (enum MESSAGE_TYPE)BAD_MSG;
    ngx_int_t len = 0;
    u_char* p = NULL;
    session_t* s = NULL;

    ngx_variable_value_t* var = get_var_ptr(r, s_mc->idx_session);
    if (!var) {
        LOG_ERROR(r->connection->log, "get session ptr failed");
        goto session_failed;
    }

    // set session_id for each pv
    len = sizeof(session_t);
    p = (u_char*)ngx_pcalloc(r->pool, len);
    if (!p) {
        LOG_ERROR(r->connection->log, "palloc for session failed");
        goto session_failed;
    }
    var->data = p;
    var->len = len;

    s = (session_t*)p;
    s->timestamp = (uint32_t)time(NULL);
    get_ip(r, &s->ip);

    if (!data->data || data->len <= 2) {
        goto session_failed;
    }

    // parse pb message from request
    // parse the first two bytes to msg type, stored by little endian
    type = *((unsigned short*)data->data);
    if (IS_BIG_ENDIAN) {
        type = ntohs(type);
    }
    s->message_type = (enum MESSAGE_TYPE)type;
    s->message_data.len = data->len - 2;
    s->message_data.data = data->data + 2;

    // parse message data by msg type, and allocate relative processor handler
    switch (s->message_type) {
        case USER_INIT_MSG:
            msg_type = s_message_type_values[USER_INIT_MSG_IDX];
            s->processor = (void*)ObjectPallocator<UserInitProcessor>::allocate(r->pool);
            break;
        case USER_REG_MSG:
            msg_type = s_message_type_values[USER_REG_MSG_IDX];
            s->processor = (void*)ObjectPallocator<UserRegisterProcessor>::allocate(r->pool);
            break;
        case USER_LOGIN_MSG:
            msg_type = s_message_type_values[USER_LOGIN_MSG_IDX];
            s->processor = (void*)ObjectPallocator<UserLoginProcessor>::allocate(r->pool);
            break;
        case USER_SET_PROF_MSG:
            msg_type = s_message_type_values[USER_SET_PROF_MSG_IDX];
            s->processor = (void*)ObjectPallocator<UserSetProfProcessor>::allocate(r->pool);
            break;
        case USER_GET_PROF_MSG:
            msg_type = s_message_type_values[USER_GET_PROF_MSG_IDX];
            s->processor = (void*)ObjectPallocator<UserGetProfProcessor>::allocate(r->pool);
            break;
        case USER_GET_ACHIEVE_MSG:
            msg_type = s_message_type_values[USER_GET_ACHIEVE_MSG_IDX];
            s->processor = (void*)ObjectPallocator<UserGetAchieveProcessor>::allocate(r->pool);
            break;
        case USER_GET_UGC_LIST_MSG:
            msg_type = s_message_type_values[USER_GET_UGC_LIST_MSG_IDX];
            s->processor = (void*)ObjectPallocator<UserGetUgcListProcessor>::allocate(r->pool);
            break;
        case USER_CHECK_REG_MSG:
            msg_type = s_message_type_values[USER_CHECK_REG_MSG_IDX];
            s->processor = (void*)ObjectPallocator<UserCheckRegProcessor>::allocate(r->pool);
            break;
        case UGC_UPLOAD_MSG:
            msg_type = s_message_type_values[UGC_UPLOAD_MSG_IDX];
            s->processor = ObjectPallocator<UgcUploadProcessor>::allocate(r->pool);
            break;
        case UGC_PULL_UGC_LIST_MSG:
            msg_type = s_message_type_values[UGC_PULL_UGC_LIST_MSG_IDX];
            s->processor = ObjectPallocator<UgcPullListProcessor>::allocate(r->pool);
            break;
        case UGC_GET_UGC_MSG:
            msg_type = s_message_type_values[UGC_GET_UGC_MSG_IDX];
            s->processor = (void*)ObjectPallocator<UgcGetUgcProcessor>::allocate(r->pool);
            break;
        case UGC_POST_UGC_MSG:
            msg_type = s_message_type_values[UGC_POST_UGC_MSG_IDX];
            s->processor = ObjectPallocator<UgcPostProcessor>::allocate(r->pool);
            break;
        case UGC_ADD_COM_MSG:
            msg_type = s_message_type_values[UGC_ADD_COM_MSG_IDX];
            s->processor = ObjectPallocator<UgcAddCommentProcessor>::allocate(r->pool);
            break;
        case UGC_GET_COM_MSG:
            msg_type = s_message_type_values[UGC_GET_COM_MSG_IDX];
            s->processor = ObjectPallocator<UgcGetCommentProcessor>::allocate(r->pool);
            break;
        case UGC_ADD_CLT_MSG:
            msg_type = s_message_type_values[UGC_ADD_CLT_MSG_IDX];
            s->processor = ObjectPallocator<UgcAddCollectionProcessor>::allocate(r->pool);
            break;
        case UGC_PLAY_CARD_MSG:
            msg_type = s_message_type_values[UGC_PLAY_CARD_MSG_IDX];
            s->processor = ObjectPallocator<UgcPlayCardProcessor>::allocate(r->pool);
            break;
        default:
            msg_type = s_message_type_values[UNKNOWN_MSG_IDX];
            goto session_failed;
    }
    if (!s->processor) {
        msg_type = s_message_type_values[UNKNOWN_MSG_IDX];
        goto session_failed;
    }
    s->ret_code = RET_SUCCESS;
    s->main_conf = s_mc;
    s->loc_conf = (ngx_http_app_loc_conf_t*)ngx_http_get_module_loc_conf(r, (*ngx_http_module_cell));
    if (NGX_ERROR == get_session_id(r, s)) {
        LOG_ERROR(r->connection->log, "get session id failed");
        return NGX_ERROR;
    }
    set_message_type(r, msg_type);
    return NGX_OK;
session_failed:
    set_message_type(r, msg_type);
    return NGX_ERROR;
}

ngx_int_t set_message(ngx_http_request_t* r, ngx_str_t* data)
{
    LOG_DEBUG(r->connection->log, "begin to process message for pv");
    
    if (NGX_ERROR == build_session(r, data)) {
        LOG_ERROR(r->connection->log, "build session failed");
        return NGX_ERROR;
    }

    // init processor handler struct
    // init function firstly will parse pb msg
    session_t* s = get_session_ptr(r);
    ngx_str_t msg_data = ngx_string(ERROR_STRING);
    BaseProcessor* processor = (BaseProcessor*)s->processor;
    if (false == processor->init(r, s)) {
        set_message_data(r, msg_data);
        LOG_ERROR(r->connection->log, "processor init error");
        return NGX_ERROR;
    } else {
        msg_data.data = (u_char*)OK_STRING;
        msg_data.len = sizeof(OK_STRING) - 1;
        set_message_data(r, msg_data);
        LOG_DEBUG(r->connection->log, "processor init ok");
    }
    LOG_DEBUG(r->connection->log, "finish to process message for pv");
    return NGX_OK;
}

ngx_int_t set_user_session(ngx_http_request_t* r, ngx_str_t* data)
{
    session_t* s = get_session_ptr(r);
    BaseProcessor* processor = (BaseProcessor*)s->processor;
    LOG_DEBUG(r->connection->log, "redis session: %V", data);

    ngx_str_t ret_str = ngx_string(ERROR_STRING);
    if (0 == ngx_strncmp(data->data, REDIS_NIL_STRING, sizeof(REDIS_NIL_STRING) - 1)) {
        set_check_session(r, ret_str);
        processor->setRetcode(RET_FAILED);
        return NGX_ERROR;
    }
    if (0 == ngx_strncmp(data->data, REDIS_ERROR_STRING, sizeof(REDIS_ERROR_STRING) - 1)) {
        set_check_session(r, ret_str);
        processor->setRetcode(RET_FAILED);
        return NGX_ERROR;
    }
    ngx_str_t session_token = processor->getSessionToken();
    LOG_DEBUG(r->connection->log, "user session: %V", &session_token);
    if (data->len != session_token.len || 0 != ngx_strncmp(data->data, session_token.data, data->len)) {
        set_check_session(r, ret_str);
        processor->setRetcode(RET_SESSION_INVALID);
        return NGX_ERROR;
    }
    ret_str.data = (u_char*)OK_STRING;
    ret_str.len = sizeof(OK_STRING) - 1;
    set_check_session(r, ret_str);
    return NGX_OK;
}

ngx_int_t get_pvlog(ngx_http_request_t* r, ngx_str_t* data)
{
    if (NULL == s_mc) {
        return NGX_ERROR;
    }
    session_t* s = get_session_ptr(r);
    if (NULL == s || NULL == s->processor) {
        return NGX_ERROR;
    }
    u_char *begin = (u_char*)ngx_palloc(r->pool, 512);
    u_char *cur = begin;
    //1-1 version
    cur = ngx_cpymem(cur, "1.0", sizeof("1.0") - 1);
    *cur++ = '';
    //2-1 timestamp
    cur = ngx_snprintf(cur, 10, "%d", s->timestamp);
    *cur++ = '';
    //3-1 client-ip
    cur = ngx_cpymem(cur, s->ip.data, s->ip.len);
    *cur++ = '';
    //4-1 session
    cur = ngx_cpymem(cur, s->session_id.data, s->session_id.len);
    *cur++ = '';
    //5-1 uri location
    cur = ngx_cpymem(cur, r->uri.data, r->uri.len);
    *cur++ = '';
    //6-1 message_type
    cur = ngx_snprintf(cur, 5, "%d", s->message_type);
    *cur++ = '';
    //7-1 ret-code
    BaseProcessor* processor = (BaseProcessor*)s->processor;
    cur = ngx_snprintf(cur, 2, "%d", processor->getRetcode());
    //8-1 session-token
    ngx_str_t session_token = processor->getSessionToken();
    cur = ngx_cpymem(cur, session_token.data, session_token.len);
    
    data->data = begin;
    data->len = cur - begin;
    processor->clear();

    return NGX_OK;
}
