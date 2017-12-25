#ifndef _USER_GET_PROF_PROCESSOR_H_
#define _USER_GET_PROF_PROCESSOR_H_

#include "app_common/BaseProcessor.h"

class UserGetProfProcessor : public BaseProcessor {
    public:
        UserGetProfProcessor();
        virtual ~UserGetProfProcessor();
    public:
        bool init(ngx_http_request_t* r, session_t* s);
        void clear();

        ngx_str_t getSessionToken() const;
        void handerSqlQueryResult(ngx_str_t* result);
        void buildResponse();
};

#endif
