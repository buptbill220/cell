#ifndef _USER_CHECK_REG_PROCESSOR_H_
#define _USER_CHECK_REG_PROCESSOR_H_

#include "app_common/BaseProcessor.h"

class UserCheckRegProcessor : public BaseProcessor {
    public:
        UserCheckRegProcessor();
        ~UserCheckRegProcessor();
    public:
        bool init(ngx_http_request_t* r, session_t* s);
        void clear();
        
        void handerSqlQueryResult(ngx_str_t* result);
        void buildResponse();
};

#endif
