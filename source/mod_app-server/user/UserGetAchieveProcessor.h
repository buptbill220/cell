#ifndef _USER_GET_ACHIEVE_PROCESSOR_H_
#define _USER_GET_ACHIEVE_PROCESSIR_H_

#include "app_common/BaseProcessor.h"

class UserGetAchieveProcessor : public BaseProcessor {
    public:
        UserGetAchieveProcessor();
        virtual ~UserGetAchieveProcessor();
    public:
        bool init(ngx_http_request_t* r, session_t* s);
        void clear();

        ngx_str_t getSessionToken() const;
        void handerAchieveResult(ngx_str_t* result);
        void buildResponse();
        void handerCacheAchieve();
};

#endif
