#ifndef _UGC_POST_PROCESSOR_H_
#define _UGC_POST_PROCESSOR_H_

#include "app_common/BaseProcessor.h"

#define UGC_ACHIEVE_TYPE_MSG    "msg"
#define UGC_ACHIEVE_TYPE_SEED   "seed"
#define USER_ACHIEVE_TYPE_FROM  "from"
#define USER_ACHIEVE_TYPE_TO    "to"

struct UgcMsgAchieve{
    uint32_t post_num;
    uint32_t pass_num;
    uint32_t alive;
    uint32_t max_depth;
    float    max_dist;
    double   total_dist;
    UgcMsgAchieve() : post_num(0), pass_num(0), 
                      alive(0), max_depth(0), 
                      max_dist(0), total_dist(0) {}
};

struct UgcSeedAchieve {
    uint32_t alive;
    uint32_t depth;
    float    dist;
    float    total_dist;
    float    latitude;
    float    longitude;
    UgcSeedAchieve() : alive(0), depth(0),
                       dist(0), total_dist(0), 
                       latitude(0), longitude(0) {}
};

typedef struct UgcMsgAchieve UgcMsgAchieveMeta;
typedef struct UgcSeedAchieve UgcSeedAchieveMeta;

class UgcPostProcessor : public BaseProcessor {
    public:
        UgcPostProcessor();
        virtual ~UgcPostProcessor();
    public:
        bool init(ngx_http_request_t* r, session_t* s);
        void clear();

        void handerPostAction();
        void handerPassAction();

        ngx_str_t getSessionToken() const;
        void handerInsertPostResult(ngx_str_t* result);
        void handerUgcSelectAchieveType(ngx_str_t* type);
        void handerUgcSelectAcieveResult(ngx_str_t* result);
        void handerUgcUpdateAchieveType(ngx_str_t* type);
        void handerUgcUpdateAchieveResult(ngx_str_t* result);
        void handerLocalSeed();
        void handerUserAchieveType(ngx_str_t* type);
        void handerUserAchieveResult(ngx_str_t* result);
        void buildResponse();
    private:
        UgcMsgAchieveMeta m_UgcMsgAchieve;
        UgcSeedAchieveMeta m_UgcSeedAchieve;
};

#endif
