/**
 *  Copyright (c) 2015 cell-app.com
 *  All rights reserved.
 *
 *  author: fangming
 *  date: 2015.02.05
 *  
 */

#ifndef _APP_LOG_H_
#define _APP_LOG_H_

#include <stdio.h>
#include <ngx_core.h>

#ifdef _APP_GTEST_
    #define LOG(level, log, format, args...) \
        fprintf(stderr, #level ": " #format "\n", ##args);
#else
    #define LOG(level, log, args...) \
        ngx_log_error(level, log, 0, ##args)
#endif
    
#define LOG_ERROR(log, args...) \
    LOG(NGX_LOG_ERR, log, ##args)

#define LOG_EMERG(log, args...) \
    LOG(NGX_LOG_EMERG, log, ##args)

#define LOG_INFO(log, args...) \
    LOG(NGX_LOG_INFO, log, ##args)

#define LOG_DEBUG(log, args...) \
    LOG(NGX_LOG_DEBUG, log, ##args)

#define LOG_WARN(log, args...) \
    LOG(NGX_LOG_WARN, log, ##args)

#define LOG_NOTICE(log, args...) \
    LOG(NGX_LOG_NOTICE, log, ##args)

#endif
