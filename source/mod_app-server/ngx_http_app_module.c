#include "app_common/global.h"
#include "app_common/app_log.h"
#include "common/reg_var.h"
#include "app_common/common_func.h"
#include "user/user_init.h"
#include "user/user_register.h"
#include "user/user_login.h"
#include "user/user_set_prof.h"
#include "user/user_get_prof.h"
#include "user/user_get_achieve.h"
#include "user/user_get_ugc_list.h"
#include "user/user_check_reg.h"
#include "ugc/ugc_upload.h"
#include "ugc/ugc_pull_ugc_list.h"
#include "ugc/ugc_get_ugc.h"
#include "ugc/ugc_post.h"
#include "ugc/ugc_add_comment.h"
#include "ugc/ugc_get_comment.h"
#include "ugc/ugc_add_collection.h"
#include "ugc/ugc_play_card.h"

#include <time.h>

static ngx_int_t ngx_http_app_init_process(ngx_cycle_t* cycle);
static void* ngx_http_app_create_loc_conf(ngx_conf_t* cf);
static void* ngx_http_app_create_main_conf(ngx_conf_t* cf);
static ngx_int_t init_session_prefix(ngx_cycle_t* log, ngx_http_app_main_conf_t* mc);

static variable_map_t app_vars[] = {
    //common vars
    {
        ngx_string("session"),
        NULL,
        NULL },
    {
        ngx_string("message"),
        NULL,
        set_message },
    {
        ngx_string("message_type"),
        NULL,
        NULL },
    {
        ngx_string("message_data"),
        NULL,
        NULL },
    {
        ngx_string("mysql_query_string"),
        NULL,
        NULL },
    {
        ngx_string("redis_query_string"),
        NULL,
        NULL },
    {
        ngx_string("user_session"),
        NULL,
        set_user_session },
    {
        ngx_string("check_session"),
        NULL,
        NULL },
    {
        ngx_string("app_server_log"),
        get_pvlog,
        NULL },

    //user vars
    //init request
    {
        ngx_string("init_mysql_query_result"),
        NULL,
        set_init_mysql_query_result },
    {
        ngx_string("init_mysql_query_status"),
        get_init_mysql_query_status,
        NULL },
    {
        ngx_string("init_mysql_insert_result"),
        NULL,
        set_init_mysql_insert_result },
    {
        ngx_string("init_resp"),
        get_init_resp,
        NULL },
    //register request
    {
        ngx_string("register_mysql_result"),
        NULL,
        set_register_mysql_result },
    {
        ngx_string("register_resp"),
        get_register_resp,
        NULL },
    //login request
    {
        ngx_string("login_mysql_query_result"),
        NULL,
        set_login_mysql_query_result },
    {
        ngx_string("login_mysql_query_status"),
        get_login_mysql_query_status,
        NULL },
    {
        ngx_string("login_resp"),
        get_login_resp,
        NULL },
    //set user profile
    {
        ngx_string("set_user_prof_mysql_result"),
        NULL,
        set_set_user_prof_mysql_result },
    {
        ngx_string("set_user_prof_resp"),
        get_set_user_prof_resp,
        NULL },
    //get user profile
    {
        ngx_string("get_user_prof_mysql_result"),
        NULL,
        set_get_user_prof_mysql_result },
    {
        ngx_string("get_user_prof_resp"),
        get_get_user_prof_resp,
        NULL },
    //get user achievement
    {
        ngx_string("cache_achievement"),
        get_cache_achievement,
        NULL },
    {
        ngx_string("get_user_achieve_result_r"),
        NULL,
        set_get_user_achieve_result_r },
    {
        ngx_string("get_user_achieve_result_status_r"),
        get_get_user_achieve_result_status_r,
        NULL },
    {
        ngx_string("get_user_achieve_result_m"),
        NULL,
        get_get_user_achieve_result_m },
    {
        ngx_string("get_user_achieve_resp"),
        get_get_user_achieve_resp,
        NULL },
    //get user ugc list
    {
        ngx_string("get_user_ugc_list_result"),
        NULL,
        set_get_user_ugc_list_result },
    {
        ngx_string("get_user_ugc_list_resp"),
        get_get_user_ugc_list_resp,
        NULL },
    //check user register
    {
        ngx_string("check_user_reg_result"),
        NULL,
        set_check_user_reg_result },
    {
        ngx_string("check_user_reg_resp"),
        get_check_user_reg_resp,
        NULL },
    //ugc vars
    {
        ngx_string("upload_mysql_result"),
        NULL,
        set_upload_mysql_result },
    {
        ngx_string("upload_mysql_result_status"),
        get_upload_mysql_result_status,
        NULL },
    {
        ngx_string("insert_message_achieve_result"),
        NULL,
        set_insert_message_achieve_result },
    {
        ngx_string("insert_message_achieve_result_status"),
        get_insert_message_achieve_result_status,
        NULL },
    {
        ngx_string("insert_message_seed_result"),
        NULL,
        set_insert_message_seed_result },
    {
        ngx_string("insert_message_seed_result_status"),
        get_insert_message_seed_result_status,
        NULL },
    {
        ngx_string("upload_get_user_achieve_result"),
        NULL,
        set_upload_get_user_achieve_result },
    {
        ngx_string("upload_get_user_achieve_result_status"),
        get_upload_get_user_achieve_result_status,
        NULL },
    {
        ngx_string("upload_resp"),
        get_upload_resp,
        NULL },
    //pull ugc list
    {
        ngx_string("pull_ugc_list_type"),
        NULL,
        set_pull_ugc_list_type },
    {
        ngx_string("pull_ugc_list_mysql_result"),
        NULL,
        set_pull_ugc_list_mysql_result },
    {
        ngx_string("pull_ugc_list_has_local_ugc"),
        get_pull_ugc_list_has_ugc,
        NULL },
    {
        ngx_string("pull_ugc_list_pull_type"),
        get_pull_ugc_pull_type,
        NULL },
    {
        ngx_string("pull_ugc_list_has_neighbor_ugc"),
        get_pull_ugc_list_has_ugc,
        NULL },
    {
        ngx_string("pull_ugc_list_has_far_ugc"),
        get_pull_ugc_list_has_ugc,
        NULL },
    {
        ngx_string("pull_ugc_list_resp"),
        get_pull_ugc_list_resp,
        NULL },
    //get ugc detail
    {
        ngx_string("get_ugc_mysql_result"),
        NULL,
        set_get_ugc_mysql_result    },
    {
        ngx_string("get_ugc_resp"),
        get_get_ugc_resp,
        NULL },
    //post ugc
    {
        ngx_string("post_ugc_mysql_result"),
        NULL,
        set_post_ugc_mysql_result   },
    {
        ngx_string("post_ugc_mysql_result_status"),
        get_post_ugc_mysql_result_status,
        NULL },
    {
        ngx_string("post_ugc_type"),
        get_post_ugc_type,
        NULL },
    {
        ngx_string("post_ugc_select_achieve_type"),
        NULL,
        set_post_ugc_select_achieve_type },
    {
        ngx_string("post_ugc_select_achieve_result"),
        NULL,
        set_post_ugc_select_achieve_result },
    {
        ngx_string("post_ugc_select_msg_achieve_result_status"),
        get_post_ugc_select_achieve_result_status,
        NULL },
    {
        ngx_string("post_ugc_selects_seed_achieve_result_status"),
        get_post_ugc_select_achieve_result_status,
        NULL },
    {
        ngx_string("post_ugc_update_achieve_type"),
        NULL,
        set_post_ugc_update_achieve_type },
    {
        ngx_string("post_ugc_update_achieve_result"),
        NULL,
        set_post_ugc_update_achieve_result },
    {
        ngx_string("post_ugc_seed_is_local"),
        get_post_ugc_seed_is_local,
        NULL },
    {
        ngx_string("post_ugc_user_achieve_type"),
        NULL,
        set_post_ugc_user_achieve_type },
    {
        ngx_string("post_ugc_user_achieve_result"),
        NULL,
        set_post_ugc_user_achieve_result },
    {
        ngx_string("post_ugc_user_from_achieve_result_status"),
        get_post_ugc_user_achieve_result_status,
        NULL },
    {
        ngx_string("post_ugc_user_to_achieve_result_status"),
        get_post_ugc_user_achieve_result_status,
        NULL },
    {
        ngx_string("post_ugc_resp"),
        get_post_ugc_resp,
        NULL },
    // add ugc comment
    {
        ngx_string("add_comment_mysql_result"),
        NULL,
        set_add_comment_mysql_result },
    {
        ngx_string("add_comment_resp"),
        get_add_comment_resp,
        NULL },
    // get comment
    {
        ngx_string("get_comment_mysql_result"),
        NULL,
        set_get_comment_mysql_result },
    {
        ngx_string("get_comment_resp"),
        get_get_comment_resp,
        NULL },
    // add collection
    {
        ngx_string("query_clt_mysql_result"),
        NULL,
        set_query_clt_mysql_result   },
    {
        ngx_string("add_collection_status"),
        get_add_collection_status,
        NULL },
    {
        ngx_string("add_clt_mysql_result"),
        NULL,
        set_add_clt_mysql_result     },
    {
        ngx_string("add_collection_resp"),
        get_add_collection_resp,
        NULL },
    // pk msg
    {
        ngx_string("pk_get_user_achieve_result"),
        NULL,
        set_pk_get_user_achieve_result},
    {
        ngx_string("pk_get_user_achieve_result_status"),
        get_pk_get_user_achieve_result_status,
        NULL },
    {
        ngx_string("pk_mysql_result"),
        NULL,
        set_pk_mysql_result},
    {
        ngx_string("pk_resp"),
        get_pk_resp,
        NULL },
    //ugc end
    {   ngx_null_string, NULL, NULL }
};

static void* ngx_reg_app_vars(ngx_conf_t* cf, ngx_command_t* cmd, void* conf)
{
    void* ret = ngx_http_register_vars(cf, cmd, conf);
    if (ret) {
        return ret;
    }
    ngx_str_t idx_vars_name[] = {
        ngx_string("session"),
        ngx_string("message_type"),
        ngx_string("message_data"),
        ngx_string("mysql_query_string"),
        ngx_string("redis_query_string"),
        ngx_string("check_session")
    };
    ngx_str_t* idx_vars = idx_vars_name;
    ngx_http_app_main_conf_t* mc = (ngx_http_app_main_conf_t*)conf;
    mc->idx_session = ngx_http_get_variable_index(cf, idx_vars++);
    mc->idx_message_type = ngx_http_get_variable_index(cf, idx_vars++);
    mc->idx_message_data = ngx_http_get_variable_index(cf, idx_vars++);
    mc->idx_mysql_query_string = ngx_http_get_variable_index(cf, idx_vars++);
    mc->idx_redis_query_string = ngx_http_get_variable_index(cf, idx_vars++);
    mc->idx_check_session = ngx_http_get_variable_index(cf, idx_vars);
    
    LOG_DEBUG(cf->cycle->log, "register vars successfully");
    return NGX_CONF_OK;
}

static ngx_command_t ngx_http_app_commands[] = {
    {
        ngx_string("reg_vars"),
        NGX_HTTP_SRV_CONF | NGX_CONF_NOARGS,
        ngx_reg_app_vars,
        NGX_HTTP_MAIN_CONF_OFFSET,
        0,
        app_vars },
    {
        ngx_string("app_version"),
        NGX_HTTP_SRV_CONF | NGX_CONF_TAKE1,
        ngx_conf_set_str_slot,
        NGX_HTTP_MAIN_CONF_OFFSET,
        offsetof(ngx_http_app_main_conf_t, app_key),
        NULL },
    {
        ngx_string("app_key"),
        NGX_HTTP_SRV_CONF | NGX_CONF_TAKE1,
        ngx_conf_set_str_slot,
        NGX_HTTP_MAIN_CONF_OFFSET,
        offsetof(ngx_http_app_main_conf_t, app_ver),
        NULL },
    {
        ngx_string("session_timeout"),
        NGX_HTTP_SRV_CONF | NGX_CONF_TAKE1,
        ngx_conf_set_num_slot,
        NGX_HTTP_MAIN_CONF_OFFSET,
        offsetof(ngx_http_app_main_conf_t, session_timeout),
        NULL },
    {
        ngx_string("location_name"),
        NGX_HTTP_LOC_CONF | NGX_CONF_TAKE1,
        ngx_conf_set_str_slot,
        NGX_HTTP_LOC_CONF_OFFSET,
        offsetof(ngx_http_app_loc_conf_t, location_name),
        NULL },
    ngx_null_command
};

static ngx_http_module_t ngx_http_app_module_ctx = {
    NULL,                           /* preconfiguration */
    NULL,                           /* postconfiguration */
    ngx_http_app_create_main_conf,  /* create main_conf */
    NULL,                           /* init main configuration */
    NULL,                           /* create server configuration */
    NULL,                           /* merge server configuration */
    ngx_http_app_create_loc_conf,   /* create location configuration */
    NULL                            /* merge server configuration */
};

ngx_module_t ngx_http_app_module = {
    NGX_MODULE_V1,
    &ngx_http_app_module_ctx,       /* module context */
    ngx_http_app_commands,          /* module directives */
    NGX_HTTP_MODULE,                /* module type */
    NULL,                           /* init master */
    NULL,                           /* init module */
    ngx_http_app_init_process,      /* init process */
    NULL,                           /* init thread */
    NULL,                           /* exit thread */
    NULL,                           /* exit process */
    NULL,                           /* exit master */
    NGX_MODULE_V1_PADDING
};

ngx_module_t* ngx_http_module_cell = &ngx_http_app_module;

static void* ngx_http_app_create_main_conf(ngx_conf_t* cf)
{
    ngx_http_app_main_conf_t* mf = (ngx_http_app_main_conf_t*)ngx_palloc(cf->pool, sizeof(ngx_http_app_main_conf_t));
    if (!mf) {
        LOG_ERROR(cf->cycle->log, "create main conf failed");
        return NGX_CONF_ERROR;
    }
    ngx_str_null(&mf->app_key);
    ngx_str_null(&mf->app_ver);
    ngx_str_null(&mf->session_id_prefix);
    mf->session_timeout = NGX_CONF_UNSET_UINT;
    mf->idx_session = NGX_CONF_UNSET_UINT;
    mf->idx_message_type = NGX_CONF_UNSET_UINT;
    mf->idx_message_data = NGX_CONF_UNSET_UINT;
    mf->idx_mysql_query_string = NGX_CONF_UNSET_UINT;
    mf->idx_redis_query_string = NGX_CONF_UNSET_UINT;
    mf->idx_check_session = NGX_CONF_UNSET_UINT;
    return mf;
}

static void* ngx_http_app_create_loc_conf(ngx_conf_t* cf)
{
    ngx_http_app_loc_conf_t* lf = (ngx_http_app_loc_conf_t*)ngx_pcalloc(cf->pool, sizeof(ngx_http_app_loc_conf_t));
    if (!lf) {
        LOG_ERROR(cf->cycle->log, "create local conf failed");
        return NGX_CONF_ERROR;
    }
    return lf;
}

static ngx_int_t ngx_http_app_init_process(ngx_cycle_t* cycle)
{
    ngx_http_app_main_conf_t* mc = (ngx_http_app_main_conf_t*)
        ngx_http_cycle_get_module_main_conf(cycle, ngx_http_app_module);
    //session timeout should be more than 1800 seconds
    if (mc->session_timeout <= 1800) {
        mc->session_timeout = 1800;
    }
    return init_session_prefix(cycle, mc);
}

static ngx_int_t init_session_prefix(ngx_cycle_t* cycle, ngx_http_app_main_conf_t* mc)
{
    srand(time(NULL));
    static u_char buffer[128];
    static u_char hostName[256];

    ngx_memzero(hostName, sizeof(hostName));
    ngx_memcpy(hostName, cycle->hostname.data, cycle->hostname.len);
    
    uint32_t ip;
    char** addrs;
    
    struct hostent* h = gethostbyname((char*)hostName);
    if (AF_INET != h->h_addrtype) {
        LOG_ERROR(cycle->log, "impossible: gethostbyname() failed");
        return NGX_ERROR;
    }
    
    addrs = h->h_addr_list;    
    if(*addrs) {
        char* raw_ip = inet_ntoa(*(struct in_addr*)*addrs);
        ip = inet_addr(raw_ip);
        if (IS_LITTLE_ENDIAN) {
            ip = htonl(ip);
        }
    } else {
        ip = (uint32_t)random();
        LOG_ERROR(cycle->log, "emergency warning: h_addr_list is null, so ip is given a random value");
        return NGX_ERROR;
    }

    u_char* pp = ngx_snprintf(buffer, sizeof(buffer), "%08xi%08xi", ip, getpid());
    mc->session_id_prefix.data = buffer;
    mc->session_id_prefix.len = 16;

    LOG_DEBUG(cycle->log, "session id prefix is %V", &mc->session_id_prefix);
    return NGX_OK;
}
