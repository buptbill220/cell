#ifndef _USER_GET_UGC_LIST_PROCESSOR_H_
#define _USER_GET_UGC_LIST_PROCESSOR_H_

#include "app_common/BaseProcessor.h"

class UserGetUgcListProcessor : public BaseProcessor {
    public:
        UserGetUgcListProcessor();
        virtual ~UserGetUgcListProcessor();
    public:
        bool init(ngx_http_request_t* r, session_t* s);
        void clear();

        ngx_str_t getSessionToken() const;
        void handerUgcListResult(ngx_str_t* result);
        void buildResponse();
};

#endif
