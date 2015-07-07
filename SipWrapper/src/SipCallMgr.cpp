#include "SipCallMgr.h"
#include "SipProviderImpl.h"
#include "SipTrace.h"
#include "ResipStackAdapterImpl.h"


/////////////////////////////////////////////////////////////////////
////SipCallMgr
SipCallMgr* SipCallMgr::s_SipCallMgr = NULL;

SipCallMgr * SipCallMgr::GetInstance()
{
    if (s_SipCallMgr == NULL)
    {
        s_SipCallMgr = new SipCallMgr();
        s_SipCallMgr->AddRef();
        SIPSDK_INFOTRACE("SipCallMgr::GetInstance, instance: " << s_SipCallMgr);
    }

    return s_SipCallMgr;
}

void SipCallMgr::Destroy()
{
    SIPSDK_INFOTRACE("SipCallMgr::Destroy, instance: " << s_SipCallMgr);
    SIPSDK_SAFE_RELEASE(s_SipCallMgr);
}

SipCallMgr::SipCallMgr() :
    mSipProvider(NULL),
    mLiveSipCalls(),
    mLiveSipCallMutex(),
    mDestroyingSipCalls(),
    mDestroyingMutex()
{
    SIPSDK_CLASSTRACE("SipCallMgr::SipCallMgr");
}

SipCallMgr::~SipCallMgr()
{
    SIPSDK_CLASSTRACE("SipCallMgr::~SipCallMgr");
    SIPSDK_SAFE_RELEASE(mSipProvider);
    
    // Destroy DestroyingCalls
    this->ProcessDestroyingCalls(true);
    
    // Destroy LiveCalls
    {
        WriteLock lock(mLiveSipCallMutex);
        for_each(this->mLiveSipCalls.begin(), this->mLiveSipCalls.end(), SIPSDKReleaseObj() );
        SipCallMap().swap(this->mLiveSipCalls);
    }
}

void SipCallMgr::AddRef()
{
    CReferenceControl::add_reference();
}

void SipCallMgr::ReleaseRef()
{
    CReferenceControl::release_reference();
}

void SipCallMgr::SetSipProvider(SipProviderImpl * provider)
{
    SIPSDK_ASSERT_RETURN_VOID(provider);
    SIPSDK_SAFE_RELEASE(mSipProvider);
    {
        provider->AddRef();
        this->mSipProvider = provider;
    }
}

CmResult SipCallMgr::NotifyCallIndication(SipCallImpl * call)
{
    SIPSDK_ASSERT_RETURN(call,                          SIPSDK_ERROR_NULL_POINTER);
    SIPSDK_ASSERT_RETURN(mSipProvider,                  SIPSDK_ERROR_NULL_POINTER);
    SIPSDK_ASSERT_RETURN(mSipProvider->mProviderSink,   SIPSDK_ERROR_NULL_POINTER);
    
    return mSipProvider->mProviderSink->OnCallIndication(call);
}

CmResult SipCallMgr::NotifyCallDestroying(SipCallImpl * call)
{
    SIPSDK_ASSERT_RETURN(call,                          SIPSDK_ERROR_NULL_POINTER);
    SIPSDK_ASSERT_RETURN(mSipProvider,                  SIPSDK_ERROR_NULL_POINTER);
    SIPSDK_ASSERT_RETURN(mSipProvider->mProviderSink,   SIPSDK_ERROR_NULL_POINTER);
    
    Lock lock(mDestroyingMutex);
    mDestroyingSipCalls.insert(call);
    
    return SIPSDK_ERROR_NOERROR;
}

SipCallImpl * SipCallMgr::CreateSipCall(CStrRef sipFromUri, CStrRef sipToUri)
{
    Data   dataID = Helper::computeCallId();
    string callID(dataID.c_str(), dataID.size());
    
    Data dataTag = Helper::computeTag(Helper::tagSize);
    string fromTag(dataTag.c_str(), dataTag.size());
    
    string toTag   = "";
        
    SipCallImpl* call = new SipCallImpl(this, sipFromUri, sipToUri, fromTag, toTag, callID, NULL);
    {
        const SessionTimerParam& sessionTimerCfg = this->mSipProvider->GetSessionTimer();
        call->SetSessionTimer(sessionTimerCfg.enabled, false, sessionTimerCfg.sessExpires);
    }
    
    this->AddCall(call);

    SIPSDK_INFOTRACE("SipCallMgr::CreateSipCall, call: " << call << ", Call-ID: " << callID <<  ", from: " << sipFromUri  << ", to: " << sipToUri);
    return call;
}

uint32 SipCallMgr::GetLiveCallCount()
{
    ReadLock lock(mLiveSipCallMutex);
    return mLiveSipCalls.size();
}

void SipCallMgr::DetectCallLiveOrDead()
{
    WriteLock lock(mLiveSipCallMutex);

    SipCallMapIter iter = mLiveSipCalls.begin();
    for(; iter != mLiveSipCalls.end(); ++ iter)
    {
        SipCallImpl* call = iter->second;
        if(!call)
            continue;
        
        if(call->IsTimeOut())
        {
            call->SetCallState(CALL_STATE_SESSION_TIME_OUT);
            call->GetSipCallSink()->OnSessionTimeout();
            continue;
        }

        if(call->NeedSendKeepAlive())
        {
            call->SendKeepAlive();
            continue;
        }
    }
}

int SipCallMgr::ProcessDestroyingCalls(bool force_delete)
{
    int i = 0;
    {
        Lock lock(mDestroyingMutex);
        
        set<SipCallImpl*>::iterator it = mDestroyingSipCalls.begin();
        for(; it != mDestroyingSipCalls.end(); )
        {
            SipCallImpl* call = *it;

            if(call->ShouldDelete() || force_delete)
            {
                this->RemoveCall(call->GetCallID());
                ++ i;
                
                mDestroyingSipCalls.erase(it ++);
            }
            else
                ++ it;
        }
    }

    return i;
}

const SessionTimerParam& SipCallMgr::GetSessionTimer() const
{
    return mSipProvider->GetSessionTimer();
}

CmResult SipCallMgr::Send(ISipMessage * msg)
{
    SIPSDK_ASSERT_RETURN(mSipProvider,  SIPSDK_ERROR_NULL_POINTER);
    SIPSDK_ASSERT_RETURN(msg,           SIPSDK_ERROR_NULL_POINTER);
    
    // Release message automatically
    auto_ptr<SipMessage> resipMsg(static_cast<ResipSipMessage *>(msg)->Sync2Resip());
    SIPSDK_ASSERT_RETURN(resipMsg.get(), SIPSDK_ERROR_MESSAGE_CONVERT_ERROR);
    
    return mSipProvider->SendMessage(resipMsg.get());
}

CmResult SipCallMgr::Process(ISipMessage * msg)
{
    SIPSDK_ASSERT_RETURN(msg, SIPSDK_ERROR_NULL_POINTER);
    CHECK_VALID_MESSAGE(msg);
    
    SIPSDK_INFOTRACE("SipCallMgr::Process, msg: " << msg->toString());
    SDKSipMessage* sdkMsg = dynamic_cast<SDKSipMessage*> (msg);
    
    if(sdkMsg->IsSipRequest())
        return this->ProcessRequest(sdkMsg);
    else
        return this->ProcessResponse(sdkMsg);
}

CmResult SipCallMgr::ProcessRequest(SDKSipMessage * msg)
{
    SIPSDK_ASSERT_RETURN(msg, SIPSDK_ERROR_NULL_POINTER);
    SipCallImpl * call = this->GetCall(msg->GetCallID());
    
    SipMethod method = msg->GetRequstMethod();
    SIPSDK_INFOTRACE("SipCallMgr::ProcessRequest, call: " << call << ", method: " << ConvertMethod2Str(method) << ", msg: " << msg);
    
    bool is_new = false;
    // Handle existing call methods
    
    // new Call/virtual Call created for the following methods
    if(!call && (
                method == SIP_METHOD_INVITE
    		 || method == SIP_METHOD_OPTIONS
    		 || method == SIP_METHOD_MESSAGE
    		 || method == SIP_METHOD_REFER
    		 || method == SIP_METHOD_PUBLISH
    		 || method == SIP_METHOD_REGISTER))
    {
        is_new = true;
        call   = this->CreateCallFromRequest(msg);
    }
    
    // Ricky, 2011/12/11 6:55:10
    // no such call leg
    if(!call)
    {
        SIPSDK_AUTO_REF<ISipMessage> response(ISipMessage::CreateSipResponse(msg));
        response->SetResponseStatusCode(481);
        
        return this->Send(response.get());
    }        
    
    if(is_new)
        call->AddRef();
    
    CmResult ret = call->ProcessRequest(msg);
    
    if(is_new)
        call->ReleaseRef();
    
    return ret;
}


CmResult SipCallMgr::ProcessResponse(SDKSipMessage * msg)
{
    SIPSDK_ASSERT_RETURN(msg, SIPSDK_ERROR_NULL_POINTER);
    SipCallImpl * call = this->GetCall(msg->GetCallID());
    
    if(!call)
    {
        SIPSDK_ERRTRACE("SipCallMgr::ProcessResponse, mismatching call for Call-ID: " << msg->GetCallID() << ", msg: " << msg);
        return SIPSDK_ERROR_INVALID_SIP_MESSAGE;
    }
    
    return call->ProcessResponse(msg);
}

SipCallImpl * SipCallMgr::CreateCallFromRequest(SDKSipMessage * msg)
{
    SIPSDK_ASSERT_RETURN(msg, NULL);

    // Compute the to tag
    if(msg->GetTo()->GetTag().empty())
    {
        SIPSDK_DETAILTRACE("SipCallMgr::CreateCallFromRequest, SIP to tag is empty.");
        
        Data dataTag = Helper::computeTag(Helper::tagSize);
        msg->GetTo()->SetTag(dataTag.c_str());
    }
    
    // 1. create new call and save it in live call cache
    string remoteUri = msg->GetFrom()->GetFrom();
    string remotetag = msg->GetFrom()->GetTag();
    string localUri  = msg->GetTo()->GetTo();
    string localTag  = msg->GetTo()->GetTag();

    SipCallImpl * call =  new SipCallImpl(
                                this,
                                remoteUri,
                                localUri,
                                remotetag,
                                localTag,
                                msg->GetCallID(),
                                msg,
                                SIPSDK_UAS);
                                
    SIPSDK_ASSERT_RETURN(call, NULL);
    {
        call->UpdateRecvKATimestamp();
        call->SetRemoteCSeqNum(msg->GetCSeq()->GetNumber());
    }
    
    return call;
}

SipCallImpl * SipCallMgr::GetCall(CStrRef callID)
{
    SipCallImpl* call = NULL;
    {
        ReadLock lock(mLiveSipCallMutex);
        
        SipCallMapIter iter = mLiveSipCalls.find(callID);
        if(iter != mLiveSipCalls.end())
            call = iter->second;
    }
    
    return call;
}

SipCallImpl * SipCallMgr::AddCall(SipCallImpl * call)
{
    SIPSDK_ASSERT_RETURN(call, NULL);
    call->AddRef();
    
    CStrRef callID = call->GetCallID();
    {
        WriteLock lock(mLiveSipCallMutex);
        mLiveSipCalls.insert(make_pair(callID, call));
    }
    
    SIPSDK_INFOTRACE("SipCallMgr::AddCall, Call-ID: " << callID << ", call: " << call);
    return call;
}

bool SipCallMgr::RemoveCall(CStrRef callID)
{
    WriteLock lock(mLiveSipCallMutex);
    
    SipCallMapIter iter = mLiveSipCalls.find(callID);
    if(iter != mLiveSipCalls.end()) 
    {
        SipCallImpl * findCall = iter->second;
        mLiveSipCalls.erase(callID);

        SIPSDK_INFOTRACE("SipCallMgr::RemoveCall, Call-ID: " << callID << ", call: " << findCall);
        
        findCall->ReleaseRef();
        return true;
    }
    
    return false;
}
