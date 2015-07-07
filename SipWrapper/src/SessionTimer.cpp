#include "SessionTimer.h"
#include "SipCallMgr.h"
#include "SipTrace.h"
/////////////////////////////////////////////////////////////////////
////SessionTimer
SessionTimer * SessionTimer::s_session_timer = NULL;

SessionTimer *SessionTimer::Instance()
{
    if (s_session_timer == NULL)
    {
        s_session_timer = new SessionTimer();
        s_session_timer->AddRef();
    }

    return s_session_timer;
}

void SessionTimer::Destroy()
{
    SIPSDK_SAFE_RELEASE(s_session_timer);
}

SessionTimer::SessionTimer() :
    mStopFlag(true)
{
    SIPSDK_CLASSTRACE("SessionTimer::SessionTimer");
}

SessionTimer::~SessionTimer()
{
    SIPSDK_CLASSTRACE("SessionTimer::~SessionTimer");
}

void SessionTimer::AddRef()
{
    CReferenceControl::add_reference();
}

void SessionTimer::ReleaseRef()
{
    CReferenceControl::release_reference();
}

bool SessionTimer::Start()
{
    SIPSDK_INFOTRACE("SessionTimer::Start");
    if (!mStopFlag)
        return false;
    mStopFlag = false;
    
    //spawn a thread
    this->run();

    return true;
}

void SessionTimer::Stop()
{
    SIPSDK_INFOTRACE("SessionTimer::Stop");
    
    mStopFlag = true;

    this->shutdown();
    this->join();
}

void SessionTimer::thread()
{
    SIPSDK_INFOTRACE("SessionTimer::thread");
    
    // Do this every 0.2 second
    while(!isShutdown())
    {
        SipCallMgr::GetInstance()->DetectCallLiveOrDead();
        usleep(200000);
    }
    
	SIPSDK_INFOTRACE("SessionTimer::thread stop.");
}



