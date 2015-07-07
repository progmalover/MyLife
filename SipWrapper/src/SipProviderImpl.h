#ifndef _SIP_SDK_PROVIDER_IMPL_H__
#define _SIP_SDK_PROVIDER_IMPL_H__

#include "sipsdk.h"
#include "SipCallImpl.h"
#include "SipCallMgr.h"
#include "SipStackAdapter.h"
#include "SipUtil.h"



class SipProviderImpl : public ISipProvider, 
                        public CReferenceControl,
                        public ThreadIf
{
    friend class SipCallMgr;
    friend class SipCallImpl;

    enum { CLEANUP_CALL_INTERVAL = 1 };

public:   
    void            AddRef();
    void            ReleaseRef();
    
    static SipProviderImpl *instance();    
    static void destroy();

    //int             OnThreadRun();
    //void            OnThreadCleanup();
    //virtual         LPCSTR type_str() const { return "SIP_SDK_PROVIDER_THREAD"; }

    virtual void    thread();
    
    virtual void    SetSipProviderSink(ISipProviderSink * sink);

    virtual void    SetTlsCertPath(string path);
    virtual void    SetTlsDomain(string domain);
    virtual bool    AddTransport(   TransportProtocol protocol,
                                    CStrRef           ipAddress, 
                                    int               port, 
                                    AddressType       type,
                                    QosType           dscpValue);
    virtual bool    IsUsingTLS();
    
    virtual bool    Start();
    virtual void    Stop();
    virtual void    SetupSessionTimer(bool runing, int sessExpires, int minSessExpires);
    const SessionTimerParam& GetSessionTimer() const;
    
    //virtual ISipCall *      CreateSipCall (CStrRef sipFrom, CStrRef sipTo);
    virtual CmResult        SendMessage(SipMessage * msg);

    // Get statistics data of stack
    virtual StackStat*      GetStackStat();

    virtual int             	GetTransportPort();
    virtual CStrRef         	GetTransportName();

    // set sip stack log path
    virtual void SetStackLogPath(string path, string filePrefix = "", StackLogLevel level = STACK_LOG_INFO);\
    
    // set sip SDK options
    virtual int SetOpt(SipOption opt, void * s, size_t n);

private:
    DISALLOW_IMPLICIT_CONSTRUCTORS(SipProviderImpl);
    ~SipProviderImpl();
    
    // Remove those calls not used
    void    CleanupCall();
   
private:
    static SipProviderImpl *s_provider;
    
    ISipProviderSink *      mProviderSink;
    SipStackAdapter *       mSipStackAdapter;
    
    AtomicBool              mStopFlag;
    SessionTimerParam       mSessionTimerParam;
    
    time_t                  mLastCleanup;
};

#endif
