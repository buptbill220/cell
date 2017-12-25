#ifndef _USER_REGISTER_PROCESSOR_H_
#define _USER_REGISTER_PROCESSOR_H_

#include "app_common/global.h"
#include "app_common/BaseProcessor.h"

class UserRegisterProcessor : public BaseProcessor {
    public:
        UserRegisterProcessor();
        virtual ~UserRegisterProcessor();
    public:
        bool init(ngx_http_request_t* r, session_t* s);
        void clear();
        void handerSqlInsertResult(ngx_str_t* result);

        ngx_str_t getSessionToken() const;
        ngx_uint_t getUid();
        void buildResponse();
};

#endif
