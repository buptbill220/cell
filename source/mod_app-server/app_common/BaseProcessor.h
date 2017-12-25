#ifndef _BASE_PROCESSOR_H_
#define _BASE_PROCESSOR_H_

#include "app_common/global.h"
#include <tr1/unordered_map>
#include <string>

class BaseProcessor {
    public:
        BaseProcessor();
        ~BaseProcessor();
    public:
        virtual bool init(ngx_http_request_t* r, session_t* s)
        {
            m_R = r;
            m_Session = s;
            return true;
        }

        virtual bool process()
        {
            return true;
        }

        virtual void clear() = 0;

        virtual ngx_str_t getSessionToken() const
        {
            ngx_str_t tmp = ngx_null_string;
            return tmp;
        }

        virtual ngx_uint_t getUid() const 
        {
            return 0;
        }

        virtual void buildResponse() { }
    public:
        bool setData(const std::string& key, void* value);

        void* getData(const std::string& key);

        void setMessageType(MESSAGE_TYPE type);

        MESSAGE_TYPE getMessageType() const;

        void setRetcode(RETCODE code);

        RETCODE getRetcode() const;

        void setRequest(const void* request);

        const void* getRequest() const;

        void setResponse(void* response);

        void* getResponse() const;
    private:
        typedef std::tr1::unordered_map<std::string, void*> HashMap;
        //used to store intermediate data
        HashMap m_HashMap; 
    protected:
        //http request r pointer
        ngx_http_request_t* m_R;
        //session_t
        session_t* m_Session;
        //message type for pb request
        MESSAGE_TYPE m_MessageType;
        //each step status
        RETCODE m_Retcode;
        //pb request data, cann't write
        const void* m_Request;
        //pb response
        void* m_Response;
};

#endif
