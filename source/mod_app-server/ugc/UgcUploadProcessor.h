#ifndef _UGC_UPLOAD_PROCESSOR_H_
#define _UGC_UPLOAD_PROCESSOR_H_

#include "app_common/global.h"
#include "app_common/BaseProcessor.h"

class UgcUploadProcessor : public BaseProcessor {
    public:
        UgcUploadProcessor();
        virtual ~UgcUploadProcessor();
    public:
        bool init(ngx_http_request_t* r, session_t* s);
        void clear();
       
        void handerInsertUgcResult(ngx_str_t* result);
        void handerInsertAchieveResult(ngx_str_t* result);
        void handerInsertSeedResult(ngx_str_t* result);
        void handerUserAchieveResult(ngx_str_t* result);
        
        ngx_str_t getSessionToken() const;
        void buildResponse();
    private:
        uint64_t ugc_id;
};

#endif
