/*------------------------------------------------------*/
/* Sip SDK definitions                                  */
/*------------------------------------------------------*/

#ifndef _SIP_SDK_DEFINITION_H__
#define _SIP_SDK_DEFINITION_H__

// For cpp test
#ifdef __CXX_TEST__
    #include "CxxTestDefs.h"
#endif 

#include <errno.h>
#include <fcntl.h>
#include <string>
#include <exception>
#include <vector>
#include <string>
#include <map>
using namespace std;

#if !defined(StrRef)
typedef string &                StrRef;
#endif

#if !defined(CStrRef)
typedef const string &          CStrRef;
#endif

#if !defined(StrContainerIter)
typedef vector<string>::iterator    StrContainerIter;
#endif

#if !defined(StrContainer)
typedef vector<string>              StrContainer;
#endif

#if !defined(CmResult)
typedef long                     CmResult;
#endif

#if !defined(uint32)
typedef unsigned long           uint32;
#endif

#if !defined(int32)
typedef long           int32;
#endif

#if !defined(int64)
typedef long long int           int64;
#endif

#if !defined(int64)
typedef unsigned long long int           uint64;
#endif

#if !defined(BYTE)
typedef unsigned char          BYTE;
#endif

#if !defined(LPCSTR)
typedef const char *           LPCSTR;
#endif

#if !defined(LPSTR)
typedef char *                  LPSTR;
#endif

#if !defined(LPVOID)
typedef void *                  LPVOID;
#endif

#if !defined(ULONG)
typedef unsigned long                  ULONG;
#endif


// transport protocol
typedef enum 
{
    SIP_TRANSPORT_TLS = 1,
    SIP_TRANSPORT_TCP,
    SIP_TRANSPORT_UDP,
} TransportProtocol;

// address type
typedef enum 
{
   IPV4,
   IPV6
} AddressType;

// sip request method
typedef enum 
{
   SIP_METHOD_INVITE= 0,
   SIP_METHOD_ACK,
   SIP_METHOD_OPTIONS,
   SIP_METHOD_BYE,
   SIP_METHOD_CANCEL,
   SIP_METHOD_REGISTER,
   SIP_METHOD_INFO,
   SIP_METHOD_UPDATE,
   SIP_METHOD_TRANSFER,
   SIP_METHOD_REFER,
   SIP_METHOD_PUBLISH,
   SIP_METHOD_SUBSCRIBE,
   SIP_METHOD_NOTIFY,
   SIP_METHOD_MESSAGE,
   SIP_METHOD_UNKNOWN
} SipMethod;

typedef enum 
{
   QOS_VALUE = 0
} SipOption;

// for DSCP/QoS support.
typedef int QosType;


const int SIP_STATUS_CODE_MIN                   = 100;
const int SIP_STATUS_CODE_MAX                   = 699;

const int SIP_STATUS_CODE_TRYING                        = 100;
const int SIP_STATUS_CODE_RINGING                       = 180;
const int SIP_STATUS_CODE_OK                            = 200;
const int SIP_STATUS_CODE_MULTIPLE_CHOICE               = 300;
const int SIP_STATUS_CODE_MOVED_PERMANENTLY             = 301;
const int SIP_STATUS_CODE_MOVED_TEMPORARILY             = 302;
const int SIP_STATUS_CODE_USE_PROXY                     = 305;
const int SIP_STATUS_CODE_BAD_REQUEST                   = 400;
const int SIP_STATUS_CODE_NOT_FOUND                     = 404;
const int SIP_STATUS_CODE_REQUEST_TIMEOUT               = 408;
const int SIP_STATUS_CODE_SESSION_INTERVAL_TOO_SMALL    = 422;
const int SIP_STATUS_CODE_CALL_LEG_NOT_EXIST            = 481;
const int SIP_STATUS_CODE_INTERNAL_SERVER_ERROR         = 500;
const int SIP_STATUS_CODE_BUSY_EVERYWHERE               = 600;

#define SIPSDK_ERROR_MODULE_BASE            10000

#define SIPSDK_ERROR_NOERROR                        (CmResult)(0)
#define SIPSDK_ERROR_NULL_POINTER                   (CmResult)(SIPSDK_ERROR_MODULE_BASE + 1)
#define SIPSDK_ERROR_IO_FAILURE                     (CmResult)(SIPSDK_ERROR_MODULE_BASE + 2)
#define SIPSDK_ERROR_INVALID_STATUS_CODE            (CmResult)(SIPSDK_ERROR_MODULE_BASE + 3)
#define SIPSDK_ERROR_INVITE_CONFLICT                (CmResult)(SIPSDK_ERROR_MODULE_BASE + 4)
#define SIPSDK_ERROR_ADD_TRANSPORT_ERROR            (CmResult)(SIPSDK_ERROR_MODULE_BASE + 5)
#define SIPSDK_ERROR_MESSAGE_CONVERT_ERROR          (CmResult)(SIPSDK_ERROR_MODULE_BASE + 6)
#define SIPSDK_ERROR_INVALID_SIP_MESSAGE            (CmResult)(SIPSDK_ERROR_MODULE_BASE + 7)
#define SIPSDK_ERROR_SEND_MSG_EXCEPTION             (CmResult)(SIPSDK_ERROR_MODULE_BASE + 8)
#define SIPSDK_ERROR_SESSION_INTERVAL_TOO_SMALL     (CmResult)(SIPSDK_ERROR_MODULE_BASE + 9)
#define SIPSDK_ERROR_UNEXPECT                       (CmResult)(SIPSDK_ERROR_MODULE_BASE + 10)

#define SIPSDK_SUCCESSED(PM_RES) (PM_RES == SIPSDK_ERROR_NOERROR)
#define SIPSDK_FAILED(PM_RES)  (PM_RES != SIPSDK_ERROR_NOERROR)
#define SIPSDK_MIN_SESSION_TIMER_SECONDS    90

// delete and set pointer to NULL
#ifndef SIPSDK_SAFE_DELETE
#define SIPSDK_SAFE_DELETE(p) \
    do \
    { \
        if(NULL != p) \
        { \
            delete p; \
            p = NULL; \
        } \
    } while(0)
#endif

// delete [] and set pointer to NULL
#ifndef SIPSDK_SAFE_DELETE_ARRAY
#define SIPSDK_SAFE_DELETE_ARRAY(p) \
    do \
    { \
        if(NULL != p) \
        {\
            delete [] p; \
            p = NULL; \
        } \
    } while(0)
#endif

// release_reference() and set pointer to NULL
#ifndef SIPSDK_SAFE_RELEASE
#define SIPSDK_SAFE_RELEASE(p) \
    do \
    { \
        if(NULL != p) \
        { \
            p->ReleaseRef(); \
            p = NULL; \
        } \
    } while(0)
#endif
#ifndef SIPSDK_SAFE_ADDREF
#define SIPSDK_SAFE_ADDREF(p) \
    {if(p) p->AddRef();}
#endif



extern "C"
{
    string      ConvertMethod2Str(SipMethod method);
    SipMethod   ConvertStr2Method(CStrRef str);
}

#endif

