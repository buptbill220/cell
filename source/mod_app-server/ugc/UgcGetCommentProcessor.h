#ifndef _UGC_GET_COM_PROCESSOR_H_
#define _UGC_GET_COM_PROCESSOR_H_

#include "app_common/BaseProcessor.h"

class UgcGetCommentProcessor : public BaseProcessor {
    public:
        UgcGetCommentProcessor();
        virtual ~UgcGetCommentProcessor();
    public:
        bool init(ngx_http_request_t* r, session_t* s);
        void clear();

        ngx_str_t getSessionToken() const;
        void handerMysqlResult(ngx_str_t* result);
        void buildResponse();
};

#endif
