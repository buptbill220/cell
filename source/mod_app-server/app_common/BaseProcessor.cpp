#include "app_common/BaseProcessor.h"

BaseProcessor::BaseProcessor() : 
    m_R(NULL),
    m_Session(NULL),
    m_MessageType(BAD_MSG),
    m_Retcode(RET_SUCCESS),
    m_Request(NULL),
    m_Response(NULL)
{
}

BaseProcessor::~BaseProcessor()
{
    m_HashMap.clear();
}

bool BaseProcessor::setData(const std::string& key, void* value)
{
    if (key.empty() || !value) {
        return false;
    }
    m_HashMap[key] = value;
    return true;
}

void* BaseProcessor::getData(const std::string& key)
{
    if (key.empty()) {
        return NULL;
    }
    HashMap::iterator it = m_HashMap.find(key);
    if (it == m_HashMap.end()) {
        return NULL;
    }
    return it->second;
}

void BaseProcessor::setMessageType(MESSAGE_TYPE type)
{
    m_MessageType = type;
}

MESSAGE_TYPE BaseProcessor::getMessageType() const
{
    return m_MessageType;
}

void BaseProcessor::setRetcode(RETCODE code)
{
    m_Retcode = code;
}

RETCODE BaseProcessor::getRetcode() const
{
    return m_Retcode;
}

void BaseProcessor::setRequest(const void* request)
{
    m_Request = request;
}

const void* BaseProcessor::getRequest() const
{
    return m_Request;
}

void BaseProcessor::setResponse(void* response)
{
    m_Response = response;
}

void* BaseProcessor::getResponse() const
{
        return m_Response;
}

