#ifndef _USER_INIT_PROCESSOR_H_
#define _USER_INIT_PROCESSOR_H_

#include "app_common/global.h"
#include "app_common/BaseProcessor.h"

class UserInitProcessor : public BaseProcessor {
    public:
        UserInitProcessor();
        ~UserInitProcessor();
    public:
        bool init(ngx_http_request_t* r, session_t* s);
        void clear();

        void handerSqlQueryResult(ngx_str_t* result);
        void handerSqlInsertResult(ngx_str_t* result);
        void buildResponse();

        ngx_uint_t getUid() const
        {
            return uid;
        }
    private:
        ngx_uint_t uid;
};

#endif
