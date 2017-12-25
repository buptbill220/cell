#ifndef _USER_SET_PROF_PROCESSOR_H_
#define _USER_SET_PROF_PROCESSOR_H_

#include "app_common/BaseProcessor.h"

class UserSetProfProcessor : public BaseProcessor {
    public:
        UserSetProfProcessor();
        virtual ~UserSetProfProcessor();
    public:
        bool init(ngx_http_request_t* r, session_t* s);
        void clear();

        void handerSqlUpdateResult(ngx_str_t* result);
        void buildResponse();

        ngx_str_t getSessionToken() const;
};

#endif
