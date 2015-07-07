#ifndef _SIP_SDK_MESSAGE_FSM_H__
#define _SIP_SDK_MESSAGE_FSM_H__

#include "sipsdk.h"
#include "SipMessageImpl.h"
#include "SipCallImpl.h"
#include "SessionTimer.h"
#include "SipUtil.h"

class SipProviderImpl;

// Sip Call Manager, it would responsible to create, update and destroy sip call
class SipCallMgr : public CReferenceControl
{
public:
    typedef map<string, SipCallImpl *>              SipCallMap;
    typedef map<string, SipCallImpl *>::iterator    SipCallMapIter;

    static SipCallMgr * GetInstance();
    static void         Destroy();

    void                AddRef();
    void                ReleaseRef();

    // If [force_delete] is true, no matter call state, just remove it
    int                 ProcessDestroyingCalls(bool force_delete = false);
    
    // Process a sip message from stack, it is the main function entry of SipCallMgr.
    // Sip provider receive sip message from stack and sent the message to here to process,
    // Manager would handle both sip request and sip response, and notify the result to user/application
    // by involking sip call sink implemented by user.
    CmResult            Process(ISipMessage * msg);
    CmResult            Send(ISipMessage * msg);

    // Set the sip provider instance
    // Manager can get provider sink object from provider instance
    void                SetSipProvider(SipProviderImpl * provider);

    // call provideSink on_call_indication
    CmResult            NotifyCallIndication(SipCallImpl * call);
    
    // Destroy the sip call
    CmResult            NotifyCallDestroying(SipCallImpl * call);
    SipCallImpl *       GetCall(CStrRef callID); 
    SipCallImpl *       AddCall(SipCallImpl * call);
    

    // Create the sip call
    SipCallImpl *       CreateSipCall(CStrRef sipFromUri, CStrRef sipToUri);

    // Count live call number
    uint32              GetLiveCallCount();

    // Detect call live or dead, and notify to user if call dead(timeout)
    void                DetectCallLiveOrDead();
    
    const SessionTimerParam& GetSessionTimer() const;
    
private:
    
    // Assign a unique call ID for new CALL
    string              GetUniqCallID();
    
    // Process request sip message
    CmResult            ProcessRequest(SDKSipMessage * msg);

    // Process response sip message
    CmResult            ProcessResponse(SDKSipMessage * msg);

    // Create a new call from OPTIONS/INVITE message
    SipCallImpl *       CreateCallFromRequest(SDKSipMessage * msg);

private:
    // look for a exist live call
    bool                RemoveCall(CStrRef callID);
    
private:
    DISALLOW_IMPLICIT_CONSTRUCTORS(SipCallMgr);
    ~SipCallMgr();
    
    static SipCallMgr*      s_SipCallMgr;
    SipProviderImpl *       mSipProvider; 
    SipCallMap              mLiveSipCalls;
    RWMutex                 mLiveSipCallMutex;
    
    set<SipCallImpl*>       mDestroyingSipCalls;
    Mutex                   mDestroyingMutex;
};

#endif
