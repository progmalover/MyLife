#include "SipProviderImpl.h"
#include "ResipStackAdapterImpl.h"
#include "SessionDescriptionImpl.h"
#include "SipTrace.h"

ISipLoggerSink* gSipSDKLoggerSink = NULL;
void SetupSipSDKLoggerSink(ISipLoggerSink * sink)
{
    if(gSipSDKLoggerSink || !sink)
        return;

    gSipSDKLoggerSink = sink;
    gSipSDKLoggerSink->AddReference();
}

void RemoveSipSDKLoggerSink()
{
    if(!gSipSDKLoggerSink)
        return;

    gSipSDKLoggerSink->ReleaseReference();
}

/////////////////////////////////////////////////////////////////////
////SipProviderImpl
SipProviderImpl * SipProviderImpl::s_provider = NULL;
//ISipLoggerSink * SipProviderImpl::s_LoggerSink = NULL;

ISipProvider* ISipProvider::GetInstance()
{
    return SipProviderImpl::instance();
    //static SipProviderImpl ss;
    //return &ss;
}

void ISipProvider::DestroyInstance()
{
    SipProviderImpl::destroy();
}

SipProviderImpl *SipProviderImpl::instance()
{
    if (s_provider == NULL)
    {
        s_provider = new SipProviderImpl();
        s_provider->AddRef();

        //if(USE_SIP_SDK_DEFAULT_TRACE)
        //{
            //set the default log trace
            //::dc_init_config("sipsdk", "./log", NULL, NULL, 1024 * 1024 * 1024ll);
            //::atexit(dc_cleanup);
            
            //INFOTRACE("SipProviderImpl::instance, ret: " << s_provider);
        //}    
    }

    return s_provider;
}

void SipProviderImpl::destroy()
{
    SIPSDK_SAFE_RELEASE(s_provider);
}

SipProviderImpl::SipProviderImpl() :
    mProviderSink(NULL),
    mSipStackAdapter(NULL),
    mStopFlag(true),
    mLastCleanup(0)
{
    SIPSDK_CLASSTRACE("SipProviderImpl::SipProviderImpl");
    mSipStackAdapter = SipStackAdapter::CreateResipStackAdapter();
    mSipStackAdapter->AddRef();

    // Modified by Sam, turn off resip Logger when startup
    // Would turn on it after application set stack log path implicit
    Log::setLevel(Log::None);
    // mSipStackAdapter->InitLog();
    
    mSessionTimerParam.enabled = false;
    ::time(&mLastCleanup);
    
}

SipProviderImpl::~SipProviderImpl()
{
    SIPSDK_CLASSTRACE("SipProviderImpl::~SipProviderImpl");
    if(mProviderSink)
    {
        mProviderSink->ReleaseReference();
        mProviderSink = NULL;
    }
    
    if(mSipStackAdapter)
    {
        mSipStackAdapter->ReleaseRef();
        mSipStackAdapter = NULL;
    }
}

void SipProviderImpl::AddRef()
{
    CReferenceControl::add_reference();
}

void SipProviderImpl::ReleaseRef()
{
    CReferenceControl::release_reference();
}

void SipProviderImpl::SetSipProviderSink(ISipProviderSink * sink)
{
    if(!sink)
        return;
        
    if(mProviderSink)
        mProviderSink->ReleaseReference();
        
    mProviderSink = sink;
    mProviderSink->AddReference();
}

void SipProviderImpl::SetTlsCertPath(string path)
{
    SIPSDK_INFOTRACE("SipProviderImpl::SetTlsCertPath, path:" << path);
    mSipStackAdapter->SetTlsCertPath(path);
}

void SipProviderImpl::SetTlsDomain(string domain)
{
    SIPSDK_INFOTRACE("SipProviderImpl::SetTlsDomain, domain:" << domain);
    mSipStackAdapter->SetTlsDomain(domain);
}
    
bool SipProviderImpl::AddTransport(
                        TransportProtocol protocol,
                        CStrRef           ipAddress,
                        int               port, 
                        AddressType       type,
                        QosType           dscpValue)
{
    if (SIPSDK_FAILED(mSipStackAdapter->AddTransport(protocol, ipAddress, port, type, dscpValue)))
        return false;

    SIPSDK_INFOTRACE("SipProviderImpl::AddTransport, protocol: " << protocol 
                                              << ", ipAddress: " << ipAddress 
                                              << ", port: "      << port 
                                              << ", type: "      << type 
                                              << ", dscp: "      << dscpValue);
    
    return true;
}

bool SipProviderImpl::IsUsingTLS()
{
    return mSipStackAdapter->IsUsingTLS();
}

bool SipProviderImpl::Start()
{
    SIPSDK_INFOTRACE("SDK was built at: " << __DATE__", "__TIME__);
    SIPSDK_INFOTRACE("SipProviderImpl::Start");
    
    if (!mStopFlag)
        return false;
    
    mStopFlag = false;
    
    if (SIPSDK_FAILED(mSipStackAdapter->Init()))
        return false;

    // Added by Sam
    SipCallMgr::GetInstance();
    SipCallMgr::GetInstance()->SetSipProvider(this);

    // spawn a thread
    this->run();

    
    // start session timer
    SessionTimer::Instance()->Start();
    
    return true;
}

void SipProviderImpl::Stop()
{
    SIPSDK_INFOTRACE("SipProviderImpl::Stop");
    
    mStopFlag = true;
    mSipStackAdapter->Shutdown();

    // stop thread
    this->shutdown();
    this->join();

    // stop session timer
    if (mSessionTimerParam.enabled)
    {
        SessionTimer::Instance()->Stop();
        SessionTimer::Destroy();
    }
    
    // Added by Sam
    SipCallMgr::Destroy();
}

void SipProviderImpl::SetupSessionTimer(bool enabled, int sessExpires, int minSessExpires)
{
    // Ricky modified logic, 2011-11-22 15:03:00
    if(minSessExpires < SIPSDK_MIN_SESSION_TIMER_SECONDS || sessExpires < SIPSDK_MIN_SESSION_TIMER_SECONDS)
    {
        SIPSDK_WARNINGTRACE("Error sessExpires or minSessExpires, the value MUST NOT lower than " << SIPSDK_MIN_SESSION_TIMER_SECONDS);
        
        if(minSessExpires < SIPSDK_MIN_SESSION_TIMER_SECONDS)
            minSessExpires = SIPSDK_MIN_SESSION_TIMER_SECONDS;
            
        if(sessExpires < SIPSDK_MIN_SESSION_TIMER_SECONDS)
            sessExpires = SIPSDK_MIN_SESSION_TIMER_SECONDS;
    }

    mSessionTimerParam.enabled          = enabled;
    mSessionTimerParam.sessExpires      = sessExpires;
    mSessionTimerParam.minSessExpires   = minSessExpires;

    SIPSDK_INFOTRACE("SipProviderImpl::SetupSessionTimer, enabled: " << mSessionTimerParam.enabled
                                                     << ", Min-SE: " << mSessionTimerParam.minSessExpires 
                                            << ", Session-Expires: " << mSessionTimerParam.sessExpires);
}

const SessionTimerParam& SipProviderImpl::GetSessionTimer() const
{
    return this->mSessionTimerParam;
}

CmResult SipProviderImpl::SendMessage(SipMessage * msg)
{
    return mSipStackAdapter->SendMsg(msg);
}

StackStat* SipProviderImpl::GetStackStat()
{
    return mSipStackAdapter->GetStackStat();
}

int SipProviderImpl::GetTransportPort()
{
    return mSipStackAdapter->GetTransportPort();
}

CStrRef SipProviderImpl::GetTransportName()
{
    return mSipStackAdapter->GetTransportName();
}

void SipProviderImpl::SetStackLogPath(string path, string filePrefix, StackLogLevel level)
{
    mSipStackAdapter->InitLog(path, filePrefix, level);
}

int SipProviderImpl::SetOpt(SipOption opt, void * s, size_t n)
{
    return mSipStackAdapter->SetOpt(opt, s, n);
}

void SipProviderImpl::CleanupCall()
{
    time_t now;
    ::time(&now);
    
    // Do this every CLEANUP_CALL_INTERVAL seconds
    if( now > (mLastCleanup + CLEANUP_CALL_INTERVAL) )
    {
        SipCallMgr::GetInstance()->ProcessDestroyingCalls();
        mLastCleanup = now;
    }
}

void SipProviderImpl::thread()
{
    SIPSDK_INFOTRACE("SipProviderImpl::thread start, this: " << this);
    SIPSDK_ASSERT_RETURN_VOID(mSipStackAdapter);

    while(!isShutdown())
    {
        ISipMessage * msg = mSipStackAdapter->RecvMsg();
        if(msg)
        {
            // handle the sip message, sip call manager would create a new sip call for new INVITE
            // and notify it to user, also notify user if receive specified sip message, like would
            // invole callback function "ISipCallSink::OnRecvACK" if it is a ACK message
            SIPSDK_AUTO_REF<ISipMessage> auto_msg(msg);
            SipCallMgr::GetInstance()->Process(auto_msg.get());
        }
        
        this->CleanupCall();
    }

    SIPSDK_INFOTRACE("SipProviderImpl::thread stop, this: " << this);
}


