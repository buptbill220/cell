#ifndef _UGC_ADD_CLT_PROCESSOR_H_
#define _UGC_ADD_CLT_PROCESSOR_H_

#include "app_common/BaseProcessor.h"

class UgcAddCollectionProcessor : public BaseProcessor { 
    public:
        UgcAddCollectionProcessor();
        virtual ~UgcAddCollectionProcessor();
    public:
        bool init(ngx_http_request_t* r, session_t* s);
        void clear();

        void handleQueryCltResult(ngx_str_t* result);
        void handleAddCltResult(ngx_str_t* result);

        ngx_str_t getSessionToken() const;
        void buildResponse();
    private:
        uint64_t clt_id;
};

#endif
