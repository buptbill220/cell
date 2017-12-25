#ifndef _UGC_PULL_LIST_PROCESSOR_H_
#define _UGC_PULL_LIST_PROCESSOR_H_

#include "app_common/BaseProcessor.h"

class UgcPullListProcessor : public BaseProcessor {
    public:
        UgcPullListProcessor();
        virtual ~UgcPullListProcessor();
    public:
        bool init(ngx_http_request_t* r, session_t* s);
        void clear();

        void handerUgcType(ngx_str_t* type);
        void handerUgcMysqlResult(ngx_str_t* result);

        ngx_str_t getSessionToken() const;
        void buildResponse();
    private:
        char** m_Neighbors;
        char* m_Geohash;
};

#endif
