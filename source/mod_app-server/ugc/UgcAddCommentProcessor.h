#ifndef _UGC_ADD_COM_PROCESSOR_H_
#define _UGC_ADD_COM_PROCESSOR_H_

#include "app_common/BaseProcessor.h"

class UgcAddCommentProcessor : public BaseProcessor {
    public:
        UgcAddCommentProcessor();
        virtual ~UgcAddCommentProcessor();
    public:
        bool init(ngx_http_request_t* r, session_t* s);
        void clear();

        ngx_str_t getSessionToken() const;
        void handleMysqlResult(ngx_str_t* result);
        void buildResponse();
    private:
        uint64_t mc_id;
};

#endif
