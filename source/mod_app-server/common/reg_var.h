/**
 * Copyright (c) 2014 Taobao.com
 * All rights reserved.
 *
 * 文件名称：reg_var.h
 * 摘要：
 *
 * 作者：jimmy.gj<jimmy.gj@taobao.com>
 * 日期：2014.10.21
 * PS:参考了zhixia代码 
 *
 * 修改摘要：
 * 修改者：
 * 修改日期：
 */
#ifndef REG_VAR_H_
#define REG_VAR_H_

#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

/*
 *    第一次读取变量时，会调用get
 *    第一次设置变量时，会调用set
 *    对于get: 需要设置变量的data & len
 *    对于set: 需要使用变量的data & len
 */
typedef ngx_int_t (*var_handle_pt)(
        ngx_http_request_t* r, 
        ngx_str_t* data  // dest buf, read or write
        );

typedef struct variable_map_s {
    ngx_str_t             name;
    var_handle_pt         get;  //read var
    var_handle_pt         set;  //write var
}variable_map_t;

// 注册变量使用
/*
struct ngx_command_s {  
    ngx_str_t       name;//模块的名称  
    ngx_uint_t      type;  
    char *(*set)(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);//指令的执行函数  
    ngx_uint_t      conf;  
    ngx_uint_t      offset; //在父指令块中的偏移  
    void            *post; //读取配置文件时可能使用的指令  
};
*/

void* ngx_http_register_vars(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);

#endif
