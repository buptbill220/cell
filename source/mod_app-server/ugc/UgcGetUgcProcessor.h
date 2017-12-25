#ifndef _UGC_GET_UGC_PROCESSOR_H_
#define _UGC_GET_UGC_PROCESSOR_H_

#include "app_common/BaseProcessor.h"

using std::tr1::unordered_map;

class UgcGetUgcProcessor : public BaseProcessor {
    public:
        UgcGetUgcProcessor();
        virtual ~UgcGetUgcProcessor();
    public:
        bool init(ngx_http_request_t* r, session_t* s);
        void clear();

        ngx_str_t getSessionToken() const;
        void handerMysqlQueryResult(ngx_str_t* result);
        void buildResponse();
    private:
        uint64_t ugc_id;
};

#endif
