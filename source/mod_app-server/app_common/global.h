/**
 *  Copyright (c) 2015 cell-app.com
 *  All rights reserved.
 *
 *  author: fangming
 *  date: 2015.02.05
 *  
 */

#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

//definition of request message type
enum MESSAGE_TYPE {
    //Bad msg
    BAD_MSG = -1,
    //User msg
    USER_INIT_MSG = 1,      //initial request msg
    USER_REG_MSG,           //register request msg
    USER_LOGIN_MSG,         //login request msg
    USER_SET_PROF_MSG,      //get user profiles msg
    USER_GET_PROF_MSG,      //set user profiles msg
    USER_GET_ACHIEVE_MSG,   //get user achivement msg
    USER_GET_UGC_LIST_MSG,  //get self or post or pass ugc list
    USER_CHECK_REG_MSG,     //check user is registered or not
    //UGC msg, start from 101 for appending user msg type
    //in the future
    UGC_UPLOAD_MSG = 101,   //upload an ugc msg
    UGC_PULL_UGC_LIST_MSG,  //pull ugc list msg
    UGC_GET_UGC_MSG,        //get ugc detail msg
    UGC_POST_UGC_MSG,       //post or pass action for a ugc message
    UGC_ADD_COM_MSG,        //add ugc comment
    UGC_GET_COM_MSG,        //get comment detail
    UGC_ADD_CLT_MSG,        //add collection
    UGC_PLAY_CARD_MSG       //play card
};

#define UNKNOWN_MESSAGE     "-1"
#define REDIS_NIL_STRING    "nil"
#define REDIS_ERROR_STRING  "ERR unknown command"
#define OK_STRING           "ok"
#define ERROR_STRING        "error"
#define EMPTY_STRING        "empty"

//black list table related defination
#define BLACKLIST_TABLE_NUM 8
#define BLACKLIST_TABLE_MASK 0x07 

//definition of retcode
enum RETCODE {
    RET_SUCCESS = 0,            //模块处理成功
    RET_INVALID_REQ,            //pb解析错误
    RET_INTERNAL_ERROR,         //模块内部错误，比如内存分配失败
    RET_MYSQL_RESULT_INVALID,   //数据库结果不是期望值
    RET_REDIS_RESULT_INVALID,   //redis结果不是期望值
    RET_SESSION_INVALID,        //session过期无效
    RET_FAILED                  //其他
};

//used to judge the cpu is little endian or big endian
#define IS_LITTLE_ENDIAN (((char)0x01020304 == 0x04))
#define IS_BIG_ENDIAN (((char)0x01020304 == 0x01))

//main configuration definition
typedef struct {
    ngx_str_t       app_key;
    ngx_str_t       app_ver;
    ngx_int_t       session_timeout;
    ngx_str_t       session_id_prefix;
    ngx_int_t       idx_session;
    ngx_int_t       idx_message_type;
    ngx_int_t       idx_message_data;
    ngx_int_t       idx_mysql_query_string;
    ngx_int_t       idx_redis_query_string;
    ngx_int_t       idx_check_session;
} ngx_http_app_main_conf_t;

//local configuration definition
typedef struct {
    ngx_str_t       location_name;
} ngx_http_app_loc_conf_t;

//session definition for each pv
typedef struct {
    uint32_t        timestamp;
    ngx_str_t       ip;
    ngx_str_t       session_id;     //session id used to record pvlog
    enum MESSAGE_TYPE    message_type;   //message type for a request msg
    ngx_str_t       message_data;   //message data
    void*           processor;      //request processor handler
    enum RETCODE    ret_code;       //hander flag
    ngx_http_app_main_conf_t* main_conf;
    ngx_http_app_loc_conf_t* loc_conf;
} session_t;

#ifdef __cplusplus
}
#endif

#endif
