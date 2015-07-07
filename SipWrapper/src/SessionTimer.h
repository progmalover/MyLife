#ifndef _SIP_SDK_SESSION_TIMER_H__
#define _SIP_SDK_SESSION_TIMER_H__

#include "sipsdk.h"
#include "SipUtil.h"

class SessionTimer : public CReferenceControl,
                          public ThreadIf
{
public:
    SessionTimer();
    virtual ~SessionTimer();
    
    void            AddRef();
    void            ReleaseRef();

    //int             OnThreadRun();
    //void            OnThreadCleanup();
    //virtual         LPCSTR type_str() const { return "SIP_SDK_SESSION_TIMER_THREAD"; }

    virtual void    thread();

    
    bool            Start();
    void            Stop();
    
    static SessionTimer*    Instance();    
    static void             Destroy();
private:
    static SessionTimer*    s_session_timer;
    bool                    mStopFlag;
};

#endif
