#ifndef _USER_LOGIN_PROCESSOR_H_
#define _USER_LOGIN_PROCESSOR_H_

#include "app_common/BaseProcessor.h"

class UserLoginProcessor : public BaseProcessor {
    public:
        UserLoginProcessor();
        virtual ~UserLoginProcessor();
    public:
        bool init(ngx_http_request_t* r, session_t* s);
        void clear();
        
        void handerSqlQueryResult(ngx_str_t* result);
        void buildResponse();
};

#endif

