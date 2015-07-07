#ifndef _SIP_SDK_TRACE_H__
#define _SIP_SDK_TRACE_H__

#include <iostream>
#include <sstream>
#include "SipProviderImpl.h"

extern ISipLoggerSink* gSipSDKLoggerSink;

#define TRACE_MAX_TRACE_LEN 10240


#define SIPSDK_INFOTRACE(str)       SIPSDK_INFO("[SIP SDK] "    << str)
#define SIPSDK_DETAILTRACE(str)     SIPSDK_DETAIL("[SIP SDK] "  << str)
#define SIPSDK_WARNINGTRACE(str)	SIPSDK_WARNING("[SIP SDK] " << str)
#define SIPSDK_ERRTRACE(str)        SIPSDK_ERR("[SIP SDK] "     << str)
#define SIPSDK_CLASSTRACE(str)      SIPSDK_DETAIL("[SIP SDK] "  << str << ", this: " << this)

#define SIPSDK_INFO(buf) \
    do { \
	std::ostringstream formator; \
       formator << buf; \
       if(gSipSDKLoggerSink) \
       { \
       	gSipSDKLoggerSink->Info(formator.str().c_str()); \
       } \
    } while (0)

#define SIPSDK_DETAIL(buf) \
    do { \
        std::ostringstream formator; \
        formator << buf; \
        if(gSipSDKLoggerSink) \
        { \
            gSipSDKLoggerSink->Detail(formator.str().c_str()); \
        } \
    } while (0)

#define SIPSDK_WARNING(buf) \
    do { \
        std::ostringstream formator; \
        formator << buf; \
        if(gSipSDKLoggerSink) \
        { \
            gSipSDKLoggerSink->Warning(formator.str().c_str()); \
        } \
    } while (0)

#define SIPSDK_ERR(buf) \
    do { \
        std::ostringstream formator; \
        formator << buf; \
        if(gSipSDKLoggerSink) \
        { \
            gSipSDKLoggerSink->Error(formator.str().c_str()); \
        } \
    } while (0)
    
/*
#define SIPSDK_INFO(str) \
    do { \
        char achFormatBuf[TRACE_MAX_TRACE_LEN]; \
        SIPSDKTextFormator formator(achFormatBuf, TRACE_MAX_TRACE_LEN); \
        char *pMsg__ = formator << str; \
        if(gSipSDKLoggerSink) \
        { \
            gSipSDKLoggerSink->Info(pMsg__); \
        } \
    } while (0)

#define SIPSDK_DETAIL(str) \
    do { \
        char achFormatBuf[TRACE_MAX_TRACE_LEN]; \
        SIPSDKTextFormator formator(achFormatBuf, TRACE_MAX_TRACE_LEN); \
        char *pMsg__ = formator << str; \
        if(gSipSDKLoggerSink) \
        { \
            gSipSDKLoggerSink->Detail(pMsg__); \
        } \
    } while (0)

#define SIPSDK_WARNING(str) \
    do { \
        char achFormatBuf[TRACE_MAX_TRACE_LEN]; \
        SIPSDKTextFormator formator(achFormatBuf, TRACE_MAX_TRACE_LEN); \
        char *pMsg__ = formator << str; \
        if(gSipSDKLoggerSink) \
        { \
            gSipSDKLoggerSink->Warning(pMsg__); \
        } \
    } while (0)

#define SIPSDK_ERR(str) \
    do { \
        char achFormatBuf[TRACE_MAX_TRACE_LEN]; \
        SIPSDKTextFormator formator(achFormatBuf, TRACE_MAX_TRACE_LEN); \
        char *pMsg__ = formator << str; \
        if(gSipSDKLoggerSink) \
        { \
            gSipSDKLoggerSink->Error(pMsg__); \
        } \
    } while (0)
*/

#define SIPSDK_ASSERT_RETURN(expr, rv) \
    if (!(expr)) { \
        SIPSDK_ERRTRACE(__FILE__ << ":" << __LINE__ << " Assert failed: " << #expr); \
        return rv; \
    } \

#define SIPSDK_ASSERT_RETURN_VOID(expr) \
    if (!(expr)) { \
        SIPSDK_ERRTRACE(__FILE__ << ":" << __LINE__ << " Assert failed: " << #expr); \
        return; \
    } \

#define CHECK_VALID_MESSAGE(msg) \
    SIPSDK_ASSERT_RETURN(msg,                                   SIPSDK_ERROR_NULL_POINTER); \
    SIPSDK_ASSERT_RETURN(msg->GetFrom(),                        SIPSDK_ERROR_NULL_POINTER); \
    SIPSDK_ASSERT_RETURN(msg->GetTo(),                          SIPSDK_ERROR_NULL_POINTER); \
    SIPSDK_ASSERT_RETURN(!msg->GetFrom()->GetTag().empty(),     SIPSDK_ERROR_INVALID_SIP_MESSAGE); \
    SIPSDK_ASSERT_RETURN(!msg->GetFrom()->toString().empty(),   SIPSDK_ERROR_INVALID_SIP_MESSAGE); \
    SIPSDK_ASSERT_RETURN(!msg->GetTo()->toString().empty(),     SIPSDK_ERROR_INVALID_SIP_MESSAGE); \
    SIPSDK_ASSERT_RETURN(!msg->GetCallID().empty(),             SIPSDK_ERROR_INVALID_SIP_MESSAGE); 
    
#endif

