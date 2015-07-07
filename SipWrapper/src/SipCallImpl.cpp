#include "SipCallImpl.h"
#include "SipProviderImpl.h"
#include "SipCallMgr.h"
#include "SipTrace.h"
#include "ResipStackAdapterImpl.h"
#include <sstream>
using std::stringstream;

static LPCSTR SipCallStateStr [] = 
{
    "CALL_STATE_START",
    "CALL_STATE_SENT_INVITE",
    "CALL_STATE_RECEIVED_INVITE",
    "CALL_STATE_RECEIVED_200",
    "CALL_STATE_RECEIVED_302",
    "CALL_STATE_SESSION_TIME_OUT",
    "CALL_STATE_COMPLETE",
    "CALL_STATE_SENT_BYE",
    "CALL_STATE_SENT_BYE_RESPONSE",
    "CALL_STATE_RECEIVED_BYE_RESPONSE",
    "CALL_STATE_DESTROYING",
    "CALL_STATE_UNKNOWN"
};

/////////////////////////////////////////////////////////////////////
////SipCallImpl

SipCallImpl::SipCallImpl(
                SipCallMgr *      callMgr, 
                CStrRef           sipFromUri, 
                CStrRef           sipToUri, 
                CStrRef           sipFromTag, 
                CStrRef           sipToTag, 
                CStrRef           sipCallID,
                ISipMessage*      firstCallMsg,
                SIPSDK_UA_Type    uaType) :
    mSipCallMgr(callMgr),
    mCallState(CALL_STATE_START),
    mSipCallSink(NULL),
    mCreateTime(0),
    mFailureCode(0),
    mUAType(uaType),
    mFromUri(sipFromUri),
    mFromTag(sipFromTag),
    mToUri(sipToUri),
    mToTag(sipToTag),
    mRemoteCSeqNum(0),
    mLocalCSeqNum(0),
    mLastMethodUsedInRequest(SIP_METHOD_UNKNOWN),
    mSessionInterval(90),
    mLocalRefresher(false),
    mFarRefresher(false),
    mLastSendKATime(0),
    mLastRecvKATime(0),
    mDestroyTime(0),
    mFirstCallMsg(firstCallMsg),
    mLocalSDP(NULL),
    mSentKeepAliveCSeqNum(),
    mLastRespCSeq(-1),
    mLastRespCode(-1)
{
    //SipCallImpl();
    SIPSDK_CLASSTRACE("SipCallImpl::SipCallImpl");

    this->SetCallID(sipCallID);
    
    if(mSipCallMgr != NULL)
    {
        // comment it because of Call object also has a reference to CallMgr
        //mSipCallMgr->AddRef();
    }

    if(mFirstCallMsg != NULL)
    {
        mFirstCallMsg->AddRef();
    }

    // Added by Sam 08/11/2011
    if(mUAType == SIPSDK_UAS)
    {
        mLocalUri = mToUri;
        mLocalTag = mToTag;
        mRemoteUri = mFromUri;
        mRemoteTag = mFromTag;
    }
    else
    {
        mLocalUri = mFromUri;
        mLocalTag = mFromTag;
        mRemoteUri = mToUri;
        mRemoteTag = mToTag;
    }
}

SipCallImpl::~SipCallImpl()
{
    SIPSDK_CLASSTRACE("SipCallImpl::~SipCallImpl");
    
    if(mSipCallSink)
        mSipCallSink->ReleaseReference();

    SIPSDK_SAFE_RELEASE(mFirstCallMsg);
    SIPSDK_SAFE_RELEASE(mLocalSDP);
}

void SipCallImpl::AddRef()
{
    CReferenceControl::add_reference();
}

void SipCallImpl::ReleaseRef()
{
    CReferenceControl::release_reference();
}

void SipCallImpl::SetSipCallSink(ISipCallSink * sink)
{
    SIPSDK_INFOTRACE("SipCallImpl::SetSipCallSink, sink: " << sink);
    SIPSDK_ASSERT_RETURN_VOID(sink);
    
    if(sink != mSipCallSink)
    {
        if(mSipCallSink)
            mSipCallSink->ReleaseReference();
            
        mSipCallSink = sink;
        mSipCallSink->AddReference();
    }
}

ISipCallSink * SipCallImpl::GetSipCallSink() const
{
    return mSipCallSink;
}

CmResult SipCallImpl::Invite(ISipMessage * request)
{
    SIPSDK_INFOTRACE("SipCallImpl::Invite, request: " << request);

    this->SetupRequest(request, SIP_METHOD_INVITE);
    this->SetSessionTimerParams(request);
    
    // Ricky, 2011-12-7 16:07:05
    // For 422 reInvite SDP
    if(request->IsContainSdp())
        this->SaveLocalSDP(request->GetSessionDescription());
    
    CmResult ret = this->SendMessage(request);
    this->SetCallState(CALL_STATE_SENT_INVITE);
    
    return ret;
}

CmResult SipCallImpl::Ringing(ISipMessage* response)
{
    SIPSDK_INFOTRACE("SipCallImpl::Ringing, response: " << response);
    SIPSDK_ASSERT_RETURN(response, SIPSDK_ERROR_NULL_POINTER);

    response->SetResponseStatusCode(SIP_STATUS_CODE_RINGING);
    this->SetupResponse(response, SIP_METHOD_INVITE);

    return this->SendMessage(response);
}

CmResult SipCallImpl::OK(ISipMessage* response)
{
    SIPSDK_INFOTRACE("SipCallImpl::OK, response: " << response << ", this: " << this);
    SIPSDK_ASSERT_RETURN(response, SIPSDK_ERROR_NULL_POINTER);

    response->SetResponseStatusCode(SIP_STATUS_CODE_OK);
    
    return this->Respond(response);
}

CmResult SipCallImpl::Respond(ISipMessage* response)
{
    SIPSDK_INFOTRACE("SipCallImpl::Respond, response: " << response << ", this: " << this);
    SIPSDK_ASSERT_RETURN(response, SIPSDK_ERROR_NULL_POINTER);
    
    int code = response->GetResponseStatusCode();
    if( code < SIP_STATUS_CODE_MIN || code > SIP_STATUS_CODE_MAX)
        return SIPSDK_ERROR_INVALID_STATUS_CODE;
    
    // UAC/UAS could save SDP here
    if(response->IsContainSdp())
        this->SaveLocalSDP(response->GetSessionDescription());
    
    this->SetupResponse(response);
    if(response->GetCSeq()->GetMethod() == SIP_METHOD_BYE && is_2xx(code))
    	this->SetCallState(CALL_STATE_SENT_BYE_RESPONSE);
    
    return this->SendMessage(response);
}

CmResult SipCallImpl::Cancel(ISipMessage * request)
{
    SIPSDK_INFOTRACE("SipCallImpl::Cancel, request: " << request);
    SIPSDK_ASSERT_RETURN(request, SIPSDK_ERROR_NULL_POINTER);

    this->SetupRequest(request, SIP_METHOD_CANCEL);
    return this->SendMessage(request);
}

CmResult SipCallImpl::ACK(ISipMessage * request)
{
    SIPSDK_INFOTRACE("SipCallImpl::ACK, request: " << request);
    CmResult ret = this->SendRequest(SIP_METHOD_ACK, request);
    
    // set the call status COMPLETE if send ACK to target peer    
    if(SIPSDK_ERROR_NOERROR == ret && mCallState == CALL_STATE_RECEIVED_200)
    {
        this->SetCallState(CALL_STATE_COMPLETE);
        this->UpdateSendKATimestamp();
    }
    
    return ret;
}

CmResult SipCallImpl::Bye(ISipMessage * request)
{
    SIPSDK_INFOTRACE("SipCallImpl::Bye, request: " << request);
    SIPSDK_ASSERT_RETURN(request, SIPSDK_ERROR_NULL_POINTER);

    this->SetupRequest(request, SIP_METHOD_BYE);
    CmResult ret = this->SendMessage(request);

    // Added by Sam 08/17/2011, update call status to DESTROY after send/receive BYE
    // Would not send keep alive message for these calls which DESTROY status
    this->SetCallState(CALL_STATE_SENT_BYE);
    return ret;
}

CmResult SipCallImpl::Options(ISipMessage * request)
{
    SIPSDK_INFOTRACE("SipCallImpl::Options, request: " << request);
    SIPSDK_ASSERT_RETURN(request, SIPSDK_ERROR_NULL_POINTER);

    this->SetupRequest(request, SIP_METHOD_OPTIONS);
    return this->SendMessage(request);
}

CmResult SipCallImpl::Info(ISipMessage * request)
{
    SIPSDK_INFOTRACE("SipCallImpl::Info, request: " << request);
    SIPSDK_ASSERT_RETURN(request, SIPSDK_ERROR_NULL_POINTER);

    this->SetupRequest(request, SIP_METHOD_INFO);
    return this->SendMessage(request);
}

CmResult SipCallImpl::Subscribe(ISipMessage * request)
{
    SIPSDK_INFOTRACE("SipCallImpl::Subscribe, request: " << request);
    SIPSDK_ASSERT_RETURN(request, SIPSDK_ERROR_NULL_POINTER);

    this->SetupRequest(request, SIP_METHOD_SUBSCRIBE);
    return this->SendMessage(request);
}

CmResult SipCallImpl::Notify(ISipMessage * request)
{
    SIPSDK_INFOTRACE("SipCallImpl::Notify, request: " << request);
    SIPSDK_ASSERT_RETURN(request, SIPSDK_ERROR_NULL_POINTER);

    this->SetupRequest(request, SIP_METHOD_NOTIFY);
    return this->SendMessage(request);
}

CmResult SipCallImpl::SendRequest(SipMethod method, ISipMessage * request)
{
    ISipMessage * msg = request;
    if(!msg)
    {
        msg = ISipMessage::CreateSipRequest();
        msg->AddRef();
        
        this->SetupRequest(msg, method);
    }
    
    CmResult ret = this->SendMessage(msg);
    if(!request && msg)
        msg->ReleaseRef();

    return ret;
}

CmResult SipCallImpl::Destroy()
{
    if(mDestroyTime > 0)
        return SIPSDK_ERROR_NOERROR;
    
    SIPSDK_INFOTRACE("SipCallImpl::Destroy(), this: " << this);

    ::time(&mDestroyTime);
    return this->mSipCallMgr->NotifyCallDestroying(this);
}

bool SipCallImpl::ShouldDelete() const
{
    if( CALL_STATE_SENT_BYE_RESPONSE == mCallState || CALL_STATE_RECEIVED_BYE_RESPONSE == mCallState)
		return true;
	
	// expired, should delete this call
	time_t now;
	::time(&now);
    if( now > (mDestroyTime + CALL_DESTROY_DELAY) )
        return true;
    
	return false;
}

CmResult SipCallImpl::SetCallFromURI(string fromURI)
{
    this->mFromUri = fromURI;
    return SIPSDK_ERROR_NOERROR;
}

CmResult SipCallImpl::SetCallToURI(string toURI)
{
    this->mToUri = toURI;
    return SIPSDK_ERROR_NOERROR;
}

SipCallState SipCallImpl::GetCallState() const
{
    return this->mCallState;
}

LPCSTR SipCallImpl::GetCallStateStr() const
{
    return SipCallStateStr[this->mCallState];
}

void SipCallImpl::SetCallState(SipCallState state)
{
    SIPSDK_INFOTRACE("SipCallImpl::SetCallState, " << this->GetCallStateStr() << " ==> " << SipCallStateStr[state] 
                                           << ",  this: " << this);
    this->mCallState = state;
}

void SipCallImpl::SetFirstCallMsg(ISipMessage* firstCallMsg)
{
    SIPSDK_ASSERT_RETURN_VOID(firstCallMsg);
    
    if(this->mFirstCallMsg)
        this->mFirstCallMsg->ReleaseRef();
    
    this->mFirstCallMsg = firstCallMsg;
    this->mFirstCallMsg->AddRef();
}

ISipMessage* SipCallImpl::GetFirstCallMsg() const
{
    return this->mFirstCallMsg;
}

CStrRef SipCallImpl::GetFromUri() const
{
    return this->mFromUri;
}

CStrRef SipCallImpl::GetToUri() const
{
    return this->mToUri;
}

void SipCallImpl::SetFromTag(string fromTag)
{
    this->mFromTag.assign(fromTag);
}

CStrRef SipCallImpl::GetFromTag() const
{
    return this->mFromTag;
}

void SipCallImpl::GenFromTag()
{
    if(!this->mFromTag.empty())
        return;

    Data dataTag = Helper::computeTag(Helper::tagSize);
    this->mFromTag.assign(dataTag.c_str(), dataTag.size());
}

void SipCallImpl::SetToTag(string toTag)
{
    this->mToTag.assign(toTag);
}

CStrRef SipCallImpl::GetToTag() const
{
    return this->mToTag;
}

void SipCallImpl::GenToTag()
{
    if(!this->mToTag.empty())
        return;
    
    Data dataTag = Helper::computeTag(Helper::tagSize);
    this->mToTag.assign(dataTag.c_str(), dataTag.size());
}

void SipCallImpl::SetLocalUri(string localUri)
{
    this->mLocalUri = localUri;
}
CStrRef SipCallImpl::GetLocalUri() const
{
    return this->mLocalUri;
}

void SipCallImpl::SetRemoteUri(string remoteUri)
{
    this->mRemoteUri = remoteUri;
}
CStrRef SipCallImpl::GetRemoteUri() const
{
    return this->mRemoteUri;
}

void SipCallImpl::SetLocalTag(string localTag)
{
    this->mLocalTag = localTag;
}
CStrRef SipCallImpl::GetLocalTag() const
{
    return this->mLocalTag;
}

void SipCallImpl::SetRemoteTag(string remoteTag)
{
    this->mRemoteTag = remoteTag;
}
CStrRef SipCallImpl::GetRemoteTag() const
{
    return this->mRemoteTag;
}

void SipCallImpl::SetCallID(string callID)
{
    SIPSDK_INFOTRACE("SipCallImpl::SetCallID, call: " << this << ", Call-ID: " << callID);
    this->mCallID.assign(callID);
}

CStrRef SipCallImpl::GetCallID() const
{
    return this->mCallID;
}

void SipCallImpl::SetRemoteCSeqNum(uint32 num)
{
    if(num > mRemoteCSeqNum)
        mRemoteCSeqNum = num;
}

uint32 SipCallImpl::GetRemoteCSeqNum() 
{
    return this->mRemoteCSeqNum;
}

uint32 SipCallImpl::GetRemoteNextCSeqNum()
{
    return ++ this->mRemoteCSeqNum;
}

void SipCallImpl::SetLocalCSeqNum(uint32 num)
{
    if(num > mLocalCSeqNum)
        mLocalCSeqNum = num;
}

uint32 SipCallImpl::GetLocalCSeqNum() 
{
    return this->mLocalCSeqNum;
}

uint32 SipCallImpl::GetLocalNextCSeqNum()
{
    return ++ this->mLocalCSeqNum;
}
    
void SipCallImpl::SetSessionTimer(bool localRefresher, bool farRefresher, uint32 sessionInterval)
{
    SIPSDK_INFOTRACE("SipCallImpl::SetSessionTimer, localRefresher: " << localRefresher << ", farRefresher: " << farRefresher << ", sessionInterval: " << sessionInterval);
    
    this->mLocalRefresher   = localRefresher;
    this->mFarRefresher     = farRefresher;
    this->mSessionInterval  = sessionInterval;
}

uint32 SipCallImpl::GetSessionInterval() const
{
    return this->mSessionInterval;
}

bool SipCallImpl::NeedSendKeepAlive()
{
    if(!mLocalRefresher || mCallState != CALL_STATE_COMPLETE)
        return false;
        
    // Start Session-Refresh Timer to mSessionInterval / 2 (recommended by RFC4028)
    if((mSessionInterval / 2 + mLastSendKATime) > ::time(NULL) )
        return false;

    SIPSDK_INFOTRACE("SipCallImpl::NeedSendKeepAlive() == true, this: " << this
                                                 <<  ", mLastSendKATime: " << TIME_PRT<>(mLastSendKATime, "%Y-%m-%d %H:%M:%S") 
                                                         << ", Interval: " << mSessionInterval); 
    return true;
}

void SipCallImpl::SendKeepAlive()
{
    SIPSDK_AUTO_REF<ISipMessage> keepAlive(this->CreateReInviteRequest());
    this->SendReInvite(keepAlive.get());
}

// handle INVITE request, Local_SDK(UAS) <= Remote(UAC)
// 1. will create a new call and notify this new call to user
// 2. notify user to hanle event "receive INVITE request"
CmResult SipCallImpl::ProcessInviteRequest(SDKSipMessage * msg)
{
    SIPSDK_ASSERT_RETURN(msg, SIPSDK_ERROR_NULL_POINTER);
    SIPSDK_INFOTRACE("SipCallImpl::ProcessInviteRequest, msg:" << msg);
    
    this->SetCallState(CALL_STATE_RECEIVED_INVITE);
    
    // handle remote INVITE session timer
    CmResult ret = this->HandleSessionTimerNegotiation(msg);
    SIPSDK_ASSERT_RETURN(SIPSDK_ERROR_NOERROR == ret, ret);
    
    // store call
    mSipCallMgr->AddCall(this);

    // 2. notify user that new call coming
    ret = mSipCallMgr->NotifyCallIndication(this);
    SIPSDK_ASSERT_RETURN(SIPSDK_ERROR_NOERROR == ret, ret);

    // 3. notify user to handle the event of receiving INVITE request
    SIPSDK_ASSERT_RETURN(mSipCallSink, SIPSDK_ERROR_NULL_POINTER);
    return mSipCallSink->OnRecvInviteRqst(this, msg);
}

// handle re-INVITE request
// 1. look for the call through [call-id]
// 2. check call status, if the status is not COMPLETE, discard it and response remote 500
// 2.                    if the status is COMOLETE, notify user to handle re-INVITE event
CmResult SipCallImpl::ProcessReInviteRequest(SDKSipMessage * msg)
{
    SIPSDK_INFOTRACE("SipCallImpl::ProcessReInviteRequest, call: " << this << ", msg:" << msg);
    CHECK_VALID_MESSAGE(msg);

    if(this->GetCallState() != CALL_STATE_COMPLETE)
    {
        SIPSDK_AUTO_REF<ISipMessage> response(ISipMessage::CreateSipResponse(msg));
        
        response->SetResponseStatusCode(SIP_STATUS_CODE_INTERNAL_SERVER_ERROR);
        response->SetResponseReason("server has not complete the previous INVITE request.");
        this->Respond(response.get());
        
        SIPSDK_INFOTRACE("SipCallImpl::ProcessReInviteRequest, this: " << this << ", conflict INVITE, respond 500, state: " << this->GetCallStateStr());
        return SIPSDK_ERROR_INVITE_CONFLICT;
    }
    
    this->UpdateRecvKATimestamp();
    this->SetLastMethodUsedInRequest(msg->GetRequstMethod());
    
    SIPSDK_ASSERT_RETURN(mSipCallSink, SIPSDK_ERROR_NULL_POINTER);
    return mSipCallSink->OnRecvInviteRqst(this, msg);
}

CmResult SipCallImpl::ProcessNonDialogRequest(SDKSipMessage * request)
{
    SIPSDK_ASSERT_RETURN(request, SIPSDK_ERROR_NULL_POINTER);
    SIPSDK_INFOTRACE("SipCallImpl::ProcessNonDialogRequest, request:" << request);
    
    // store call
    mSipCallMgr->AddCall(this);
    
    // notify user new call coming, user need set the call sink for this call
    CmResult ret = mSipCallMgr->NotifyCallIndication(this);
    SIPSDK_ASSERT_RETURN(SIPSDK_ERROR_NOERROR == ret, ret);
    
    // notify user to handle OPTIONS request event
    SIPSDK_ASSERT_RETURN(mSipCallSink, SIPSDK_ERROR_NULL_POINTER);
    
    switch(request->GetRequstMethod())
    {
    case SIP_METHOD_OPTIONS:
        ret = mSipCallSink->OnRecvOptionsRqst(this, request);
        break;

    case SIP_METHOD_SUBSCRIBE:
        ret = mSipCallSink->OnRecvSubscribeRqst(this, request);
        break;
    
    // 405 Method Not Allowed
    default:
        return this->RespondError(request, 405);

// Not implemented methods
/*
    case SIP_METHOD_REGISTER:
        ret = mSipCallSink->OnRecvRegisterRqst(this, msg);
        break;
        
    case SIP_METHOD_MESSAGE:
        ret = mSipCallSink->OnRecvMessageRqst(this, msg);
        break;
      
    case SIP_METHOD_REFER:
        ret = mSipCallSink->OnRecvReferRqst(this, msg);
        break;

        
    case SIP_METHOD_PUBLISH:
        ret = mSipCallSink->OnRecvPublishRqst(this, msg);
        break;
*/        
    }
    
    this->Destroy();
    return SIPSDK_ERROR_NOERROR;
}

// handle non-INVITE request
// 1. look for the call through [call-id, from tag, to tag]
// 2. check call status, if the status is not COMPLETE, discard it and response remote 500
// 2.                    if the status is COMOLETE, notify user to handle re-INVITE event

CmResult SipCallImpl::ProcessNonInviteRequset(SDKSipMessage * msg)
{
    SIPSDK_INFOTRACE("SipCallImpl::ProcessNonInviteRequset, call: " << this << ", msg:" << msg);
    CHECK_VALID_MESSAGE(msg);

    SIPSDK_ASSERT_RETURN(mSipCallSink, SIPSDK_ERROR_NULL_POINTER);

    SipMethod method = msg->GetRequstMethod();
    if (method == SIP_METHOD_ACK && mCallState == CALL_STATE_RECEIVED_INVITE)
    {
        mSipCallSink->OnRecvACKRqst(this, msg);

        // set the call status COMPLETE if receive ACK from target peer
        this->SetCallState(CALL_STATE_COMPLETE);
        this->UpdateSendKATimestamp();
    }
    else if (method == SIP_METHOD_ACK && mCallState == CALL_STATE_COMPLETE){
        SIPSDK_INFOTRACE("we should UpdateSendKATimestamp and notify user");
        this->OnRecvACKRqst(this, msg);
        this->UpdateSendKATimestamp();
    }
    else if (method == SIP_METHOD_BYE)
    {
        mSipCallSink->OnRecvByeRqst(this, msg);

        // Added by Sam 08/17/2011, update call status to DESTROY after send/receive BYE
        // Would not send keep alive message for these calls which DESTROY status
        this->Destroy();
    }
    else if (method == SIP_METHOD_CANCEL)
        mSipCallSink->OnRecvCancelRqst(this, msg);
        
    else if (method == SIP_METHOD_NOTIFY)
        mSipCallSink->OnRecvNotifyRqst(this, msg);
        
    else if (method == SIP_METHOD_INFO)
        mSipCallSink->OnRecvInfoRqst(this, msg);
        
    else
    {
        SIPSDK_ERRTRACE("SipCallImpl::ProcessNonInviteRequset, the request method is not supported: " << msg->GetRequstMethod());
        
        // 405 Method Not Allowed
        return this->RespondError(msg, 405);
    }
    
    return SIPSDK_ERROR_NOERROR;
}

CmResult SipCallImpl::HandleSessionTimerNegotiation(SDKSipMessage * inMsg)
{
    // Ricky, 2011-12-12 16:09:52
    // Session timer disable, would not negotiate again
    {
        mFarRefresher   = false;
        mLocalRefresher = false;
    }

    const SessionTimerParam& sessionTimerCfg = mSipCallMgr->GetSessionTimer();
    if(!sessionTimerCfg.enabled)
        return SIPSDK_ERROR_NOERROR;

    ResipSipMessage * adapterMsg = static_cast<ResipSipMessage *>(inMsg);
    SIPSDK_ASSERT_RETURN(adapterMsg, SIPSDK_ERROR_NULL_POINTER);
    
    SipMessage * msg = adapterMsg->GetResipMessage();
    SIPSDK_ASSERT_RETURN(msg, SIPSDK_ERROR_NULL_POINTER);
    
    uint32 useSessExpires  = sessionTimerCfg.sessExpires;   
    bool   localRefresh    = false;
    bool   farRefresh      = false;
    
    if(msg->exists(h_Supporteds) && msg->header(h_Supporteds).find(Token(Symbols::Timer)))
    {
        if(msg->exists(h_SessionExpires))
        {
            // Use Session Interval requested by remote - if none then use local settings
            useSessExpires = msg->header(h_SessionExpires).value();
            if(useSessExpires < sessionTimerCfg.minSessExpires)
            {
                this->Respond422(inMsg, sessionTimerCfg.minSessExpires);
                
                // Ricky modified, 2011-12-9 17:21:36
                // Nogotiated in the same dialog
                return SIPSDK_ERROR_SESSION_INTERVAL_TOO_SMALL;
            }
            
            // Ricky, 2011-12-12 16:16:07
            // Only far side(UAC) clearly declare it would do refresh, we would allow remote refresh
            if(msg->header(h_SessionExpires).exists(p_refresher))
                farRefresh = (msg->header(h_SessionExpires).param(p_refresher) == Data("uac"));
        }
    }
    
    // If far end doesn't support then refresher must be local
    if(!farRefresh)
        localRefresh = true;
    
    SIPSDK_INFOTRACE("SipCallImpl::HandleSessionTimerNegotiation, this: " << this 
                                                          << ", refresher: " << localRefresh 
                                                           << ", interval: " << useSessExpires);
    
    this->SetSessionTimer(localRefresh, farRefresh, useSessExpires);
    return SIPSDK_ERROR_NOERROR;
}

CmResult SipCallImpl::Respond422(SDKSipMessage * rqst, uint32 minSE)
{
    // response 422
    SIPSDK_ASSERT_RETURN(rqst, SIPSDK_ERROR_NULL_POINTER);
    
    ResipSipMessage * adapterMsg = static_cast<ResipSipMessage*>(ISipMessage::CreateSipResponse(rqst));
    SIPSDK_AUTO_REF<ResipSipMessage> resp422(adapterMsg);

    resp422->SetMinSE(minSE);
    resp422->SetResponseStatusCode(422);

    return this->Respond(resp422.get());
}

CmResult SipCallImpl::RespondError(SDKSipMessage * rqst, int code)
{
    SIPSDK_ASSERT_RETURN(rqst, SIPSDK_ERROR_NULL_POINTER);
    
    // response code
    ResipSipMessage * adapterMsg = static_cast<ResipSipMessage*>(ISipMessage::CreateSipResponse(rqst));
    SIPSDK_AUTO_REF<ResipSipMessage> resp(adapterMsg);

    resp->SetResponseStatusCode(code);
    return this->Respond(resp.get());
}

CmResult SipCallImpl::SendReInvite(ISipMessage * keepAlive)
{
    SIPSDK_ASSERT_RETURN(keepAlive, SIPSDK_ERROR_NULL_POINTER);
        
    uint32 cseqNum = keepAlive->GetCSeq()->GetNumber();
    CmResult ret = this->SendMessage(keepAlive);
    
    // store the re-INVITE cseq num, will send ACK automatically refer to this num
    if(ret == SIPSDK_ERROR_NOERROR)
    {
        this->UpdateSendKATimestamp();
        this->AddTimerCSeq(cseqNum);
    }
        
    return ret;
}

CmResult SipCallImpl::SendMessage(ISipMessage * msg)
{
    SIPSDK_ASSERT_RETURN(msg, SIPSDK_ERROR_NULL_POINTER);
    return mSipCallMgr->Send(msg);
}

CmResult SipCallImpl::ProcessRequest(SDKSipMessage * request)
{
    SIPSDK_ASSERT_RETURN(request, SIPSDK_ERROR_NULL_POINTER);
    SipMethod method = request->GetRequstMethod();

    // record the method in lasted received request, 
    // it will be used in CSeq.Method field when user response without setting CSeq.Method
    this->SetLastMethodUsedInRequest(method);
    this->SetRemoteCSeqNum(request->GetCSeq()->GetNumber());
    
    switch(method)
    {
    case SIP_METHOD_INVITE:
        // a reINVITE Request for Session Timer refresh
        if(mSipCallMgr->GetCall(mCallID))
            return this->ProcessReInviteRequest(request);

        return this->ProcessInviteRequest(request);
        
    case SIP_METHOD_OPTIONS:
    case SIP_METHOD_MESSAGE:
    case SIP_METHOD_REFER:
    case SIP_METHOD_PUBLISH:
    case SIP_METHOD_SUBSCRIBE:
        return this->ProcessNonDialogRequest(request);
        
    default:
        // 481, Not find related dialog for following in-Dialog methods
        if(!mSipCallMgr->GetCall(mCallID))
            return this->RespondError(request, 481);
        
        return this->ProcessNonInviteRequset(request);    
    }
    
    return SIPSDK_ERROR_NOERROR;
}

CmResult SipCallImpl::ProcessResponse(SDKSipMessage * msg)
{
    SIPSDK_ASSERT_RETURN(msg, SIPSDK_ERROR_NULL_POINTER);
    CHECK_VALID_MESSAGE(msg);
    
    int statusCode = msg->GetResponseStatusCode();
    {
        mLastRespCSeq  = msg->GetCSeq()->GetNumber();
        mLastRespCode  = statusCode;
    }
    
    if(statusCode < SIP_STATUS_CODE_MIN || statusCode > SIP_STATUS_CODE_MAX)
    {
        SIPSDK_ERRTRACE("SipCallImpl::ProcessResponse, this: " << this << ", cannot handle status code: " << statusCode);
        return SIPSDK_ERROR_INVALID_STATUS_CODE;
    }
    
    if(is_2xx(statusCode)) // 200
        return this->ProcessResponse200(msg);
    
    switch(statusCode)
    {
    case SIP_STATUS_CODE_TRYING:    // 100
        return mSipCallSink->OnRecvTrying(this, msg);
    
    case SIP_STATUS_CODE_RINGING:   // 180
        return mSipCallSink->OnRecvRinging(this, msg);
     
    case SIP_STATUS_CODE_MOVED_TEMPORARILY: // 302
        return this->ProcessResponse302(msg);

    case SIP_STATUS_CODE_SESSION_INTERVAL_TOO_SMALL:    // 422
        return this->ProcessResponse422(msg);

    case SIP_STATUS_CODE_REQUEST_TIMEOUT:               // 408
    case SIP_STATUS_CODE_CALL_LEG_NOT_EXIST:            // 481
        mFailureCode = statusCode;
        break;
    }

    if(msg->GetCSeq()->GetMethod() == SIP_METHOD_INVITE )
        return mSipCallSink->OnRecvInviteResp(this, msg);
            
    return mSipCallSink->OnRecvResp(this, msg);
}

CmResult SipCallImpl::ProcessResponse200(SDKSipMessage * response)
{
    SIPSDK_ASSERT_RETURN(response, SIPSDK_ERROR_NULL_POINTER);
    
    SipMethod method = response->GetCSeq()->GetMethod(); 
    switch(method)
    {
    case SIP_METHOD_INVITE:
    case SIP_METHOD_UPDATE:
        this->UpdateRecvKATimestamp();
        
        // if receive response of INVITE for session timer, sent the ACK automatically
        // otherwise, notify user to handle this event
        if(this->IsKeepAliveResponse(response->GetCSeq()->GetNumber()))
            return this->SendKeepAliveACK(response);
        
        // Initial INVITE confirmed
        else if( SIP_METHOD_INVITE == method )
        {
            this->SetCallState(CALL_STATE_RECEIVED_200);
            return mSipCallSink->OnRecvInviteResp(this, response);
        }
        
        break;
    
    case SIP_METHOD_BYE:
        this->SetCallState(CALL_STATE_RECEIVED_BYE_RESPONSE);
        this->Destroy();
        break;
    }

    return mSipCallSink->OnRecvResp(this, response);
}

CmResult SipCallImpl::ProcessResponse302(SDKSipMessage * response)
{
    SIPSDK_ASSERT_RETURN(response, SIPSDK_ERROR_NULL_POINTER);
    this->SetCallState(CALL_STATE_RECEIVED_302);
    
    // Handle INVITE 302 response
    if( response->GetCSeq()->GetMethod() == SIP_METHOD_INVITE )
        return mSipCallSink->OnRecvInviteResp(this, response);
    
    // Could be REGISTER 302 response
    return mSipCallSink->OnRecvResp(this, response);
}

CmResult SipCallImpl::ProcessResponse422(SDKSipMessage * response)
{
    SIPSDK_ASSERT_RETURN(response, SIPSDK_ERROR_NULL_POINTER);
    this->RemoveTimerCSeq(response->GetCSeq()->GetNumber());
    
    SIPSDK_AUTO_REF<ISipMessage> reInvite(this->CreateReInviteRequest());
    
    // RFC 4028, The value of the Min-SE header field present in a session refresh
    // request MUST be the largest value among all Min-SE header field
    // values returned in all 422 responses or received in session refresh requests 
    {
        const SessionTimerParam& sessionTimerCfg = mSipCallMgr->GetSessionTimer();
        uint32 minSE     = resipMax(sessionTimerCfg.minSessExpires, response->GetMinSE());
        mSessionInterval = resipMax(minSE, mSessionInterval);
        
        static_cast<SDKSipMessage*>(reInvite.get())->SetMinSE(minSE);
        static_cast<SDKSipMessage*>(reInvite.get())->SetSessionExpires(mSessionInterval);
    }
    
    return this->SendReInvite(reInvite.get());
}

bool SipCallImpl::IsTimeOut()
{
    if(this->mCallState != CALL_STATE_COMPLETE )
        return false;
        
    // Session Timer is not enabled
    if( 0 == mSessionInterval || (!mFarRefresher && !mLocalRefresher))
        return false;
    
    // 408, 481 error code handling
    if( SIP_STATUS_CODE_REQUEST_TIMEOUT    != mFailureCode && 
        SIP_STATUS_CODE_CALL_LEG_NOT_EXIST != mFailureCode )
    {       
        // Session-Expiration mSessionInterval - BYE should be sent a minimum of 32 and one third of the SessionInterval,
        // seconds before the session expires (recommended by RFC4028)
        if( mLastRecvKATime + mSessionInterval - resipMin((uint32)32, mSessionInterval / 3) > ::time(NULL) )
            return false;
    }

    SIPSDK_INFOTRACE("SipCallImpl::IsTimeOut() == true, this: " << this
                                         <<  ", mLastSendKATime: " << TIME_PRT<>(mLastSendKATime, "%Y-%m-%d %H:%M:%S") 
                                                 << ", Interval: " << mSessionInterval
                                             << ", mFailureCode: " << mFailureCode); 
    return true;
}

void SipCallImpl::UpdateRecvKATimestamp()
{
    SIPSDK_INFOTRACE("SipCallImpl::UpdateRecvKATimestamp, this: " << this);
    ::time(&mLastRecvKATime);
}

void SipCallImpl::UpdateSendKATimestamp()
{
    SIPSDK_INFOTRACE("SipCallImpl::UpdateSendKATimestamp, this: " << this);
    ::time(&mLastSendKATime);
}

CmResult SipCallImpl::SendKeepAliveACK(ISipMessage* inviteResp)
{
    SIPSDK_DETAILTRACE("SipCallImpl::SendKeepAliveACK, this: " << this);

    SIPSDK_AUTO_REF<ISipMessage> ackMsg(this->CreateKeepAliveACK(inviteResp));
    CmResult ret = this->SendMessage(ackMsg.get());
        
    // store the re-INVITE cseq num, will send ACK automatically refer to this num
    if(ret == SIPSDK_ERROR_NOERROR)
        this->RemoveTimerCSeq(inviteResp->GetCSeq()->GetNumber());
    
    return ret;
}

void SipCallImpl::SetLastMethodUsedInRequest(SipMethod method)
{
    this->mLastMethodUsedInRequest = method;
}

void SipCallImpl::SetUAType(SIPSDK_UA_Type uaType)
{
    this->mUAType = uaType;
}

SIPSDK_UA_Type SipCallImpl::GetUAType() const
{
    return this->mUAType;
}

void SipCallImpl::SaveLocalSDP(ISessionDescription* sdp)
{
    SIPSDK_INFOTRACE("SipCallImpl::SaveLocalSDP, this: " << this << ", sdp: " << sdp);
    SIPSDK_ASSERT_RETURN_VOID(sdp);

    SIPSDK_DETAILTRACE("SipCallImpl::SaveLocalSDP, SDP: \n\n" << sdp->toString());
    SIPSDK_SAFE_RELEASE(mLocalSDP);

    mLocalSDP = sdp;
    SIPSDK_SAFE_ADDREF(mLocalSDP)
}

void SipCallImpl::SetupRequest(ISipMessage * request, SipMethod method)
{
    SIPSDK_ASSERT_RETURN_VOID(request);

    mLastMethodUsedInRequest = method;

    SDKSipMessage* msg = static_cast<SDKSipMessage*>(request);
    msg->SetIsSipRequest(true);
    
    // Method
    request->SetRequstMethod(method);
    
    // CSeq
    {
        request->GetCSeq()->SetMethod(method);
        if(method == SIP_METHOD_ACK)
        {
            int localSeq = this->GetLocalCSeqNum();
            request->GetCSeq()->SetNumber(localSeq);
        }
        else
            request->GetCSeq()->SetNumber(this->GetLocalNextCSeqNum());
    }

 
    // Via, Contact, Call-ID
    this->SetCommonFields(request);
    
    // From, To, URI
    if(this->mUAType == SIPSDK_UAC)
    {
        // if UAC, the remote tag is allowed to be empty
        // make sure the local tag not empty
        this->GenFromTag();

        if(request->GetRequestURI().empty())
            request->SetRequestURI(this->GetToUri());
        
        request->GetFrom()->SetFrom(this->GetFromUri());
        request->GetFrom()->SetTag(this->GetFromTag());
        request->GetTo()->SetTo(this->GetToUri());
        request->GetTo()->SetTag(this->GetToTag());
        
    }
    else if(this->mUAType == SIPSDK_UAS)
    {   
        // if UAS, the remote tag should not be empty
        // the local tag is allowed to be empty
        // use the INVITE::Contact as request URI
        ISipMessage* inviteMsg = this->GetFirstCallMsg();
        
        ContactList contactList;
        inviteMsg->GetContactList(contactList);

        if(request->GetRequestURI().empty())
        {
            if(!contactList.empty())
                request->SetRequestURI(contactList.front()->GetContact());
            else
                request->SetRequestURI(this->GetFromUri());
        }
        
        request->GetFrom()->SetFrom(this->GetToUri());
        request->GetFrom()->SetTag(this->GetToTag());
        request->GetTo()->SetTo(this->GetFromUri());
        request->GetTo()->SetTag(this->GetFromTag());
    }
    else
        SIPSDK_ERRTRACE("SipCallImpl::SetupRequest, unknow UA type: " << this->mUAType);
}

void SipCallImpl::SetupResponse(ISipMessage * response, SipMethod method)
{
    SIPSDK_ASSERT_RETURN_VOID(response);

    SDKSipMessage* msg = static_cast<SDKSipMessage*>(response);
    msg->SetIsSipRequest(false);

    // comment: From, To & Call-ID should not be empty, both values MUST been filled 
    // in ISipMessage::CreateSipResponse(ISipMessage* request)
    if(response->GetFrom()->GetFrom().empty())
    {
        SIPSDK_WARNINGTRACE("should not print here");
        response->GetFrom()->SetFrom(this->GetFromUri());
        response->GetFrom()->SetTag(this->GetFromTag());
    }

    if(response->GetTo()->GetTo().empty())
    {
        SIPSDK_WARNINGTRACE("should not print here");
        this->GenToTag();
        response->GetTo()->SetTo(this->GetToUri());
        response->GetTo()->SetTag(this->GetToTag());
    }

    // update from tag, to tag if it is empty
    if(this->GetFromTag().empty())
        this->SetFromTag(response->GetFrom()->GetTag());
    
    if(this->GetToTag().empty())
        this->SetToTag(response->GetTo()->GetTag());

    if(response->GetCallID().empty())
        response->SetCallID(this->GetCallID());

    // handle CSeq
    if(response->GetCSeq()->GetMethod() == SIP_METHOD_UNKNOWN)
    {
        if(method != SIP_METHOD_UNKNOWN)
            response->GetCSeq()->SetMethod(method);
        if(this->mLastMethodUsedInRequest != SIP_METHOD_UNKNOWN)
            response->GetCSeq()->SetMethod(this->mLastMethodUsedInRequest);

        response->GetCSeq()->SetNumber(this->GetRemoteCSeqNum());
    }


    // response should hold the Session-Expires;refresher=uas header
    if(response->GetCSeq()->GetMethod() == SIP_METHOD_INVITE 
    && is_2xx(response->GetResponseStatusCode())
    && (mLocalRefresher || mFarRefresher))
    {
        SDKSipMessage* tmp = static_cast<SDKSipMessage*>(response);        
        tmp->SetSessionExpires(this->GetSessionInterval());
        
        StrContainer supportedList;
        supportedList.push_back("timer");
        tmp->SetSupportedList(supportedList);
        
        if(mLocalRefresher)
            tmp->SetRefresher(this->mUAType);
        else
            tmp->SetRefresher(this->mUAType == SIPSDK_UAC ? SIPSDK_UAS : SIPSDK_UAC);
        
        if(mFarRefresher)
        {
            StrContainer requiredList;
            requiredList.push_back("timer");
            tmp->SetRequireList(requiredList);
        }
    }
}

ISipMessage* SipCallImpl::CreateReInviteRequest()
{
    ISipMessage* keepAlive = ISipMessage::CreateSipRequest(); 

    keepAlive->SetRequstMethod(SIP_METHOD_INVITE);
    keepAlive->GetCSeq()->SetMethod(SIP_METHOD_INVITE);
    keepAlive->GetCSeq()->SetNumber(this->GetLocalNextCSeqNum());
    keepAlive->SetSessionDescription(mLocalSDP);
    
    if(this->mUAType == SIPSDK_UAC)
    {
        keepAlive->SetRequestURI(this->GetToUri());
        keepAlive->GetFrom()->SetFrom(this->GetFromUri());
        keepAlive->GetFrom()->SetTag(this->GetFromTag());
        keepAlive->GetTo()->SetTo(this->GetToUri());
        keepAlive->GetTo()->SetTag(this->GetToTag());
    }
    else if(this->mUAType == SIPSDK_UAS)
    {   
        ISipMessage* inviteMsg = this->mFirstCallMsg;

        ContactList contactList;
        if(inviteMsg)
            inviteMsg->GetContactList(contactList);

        // use the first contact as request URI, consider multiple contacts ??? 
        if(!contactList.empty())
            keepAlive->SetRequestURI(contactList.front()->GetContact());
        else
            keepAlive->SetRequestURI(this->GetFromUri());
        
        keepAlive->GetFrom()->SetFrom(this->GetToUri());
        keepAlive->GetFrom()->SetTag(this->GetToTag());
        keepAlive->GetTo()->SetTo(this->GetFromUri());
        keepAlive->GetTo()->SetTag(this->GetFromTag());
    }
    else
        SIPSDK_ERRTRACE("SipCallImpl::CreateReInviteRequest, unknow UA type: " << this->mUAType);
    
    this->SetCommonFields(keepAlive);
    
    // RFC 4028 setup
    {
        // RFC 4028, A UAC that supports the session timer extension defined here MUST
        // include a Supported header field in each request
        StrContainer supportedList;
        supportedList.push_back("timer");
        keepAlive->SetSupportedList(supportedList);
        
        SDKSipMessage * sdkKeepAlive = static_cast<SDKSipMessage*>(keepAlive);
            
        // RFC 4028, In a session refresh request sent within a dialog with an active
        // session timer, the Session-Expires header field SHOULD be present.
        sdkKeepAlive->SetSessionExpires(this->GetSessionInterval());
        
        // RFC 4028,If the session refresh request is not the initial one, it is
        // RECOMMENDED that the refresher parameter be set to 'uac' if the
        // element sending the request is currently performing refreshes, and to
        // 'uas' if its peer is performing the refreshes.  
        sdkKeepAlive->SetRefresher(this->mUAType);
    }
    
    return keepAlive;
}

ISipMessage* SipCallImpl::CreateKeepAliveACK(ISipMessage* inviteResp)
{
    ISipMessage* ack = ISipMessage::CreateSipRequest(); 

    ack->SetRequstMethod(SIP_METHOD_ACK);
    ack->GetCSeq()->SetMethod(SIP_METHOD_ACK);
    ack->GetCSeq()->SetNumber(inviteResp->GetCSeq()->GetNumber());

    ack->GetFrom()->SetFrom(inviteResp->GetFrom()->GetFrom());
    ack->GetFrom()->SetTag(inviteResp->GetFrom()->GetTag());
    ack->GetTo()->SetTo(inviteResp->GetTo()->GetTo());
    ack->GetTo()->SetTag(inviteResp->GetTo()->GetTag());
    
    this->SetCommonFields(ack);
    
    if(this->mUAType == SIPSDK_UAC)
    {
        ack->SetRequestURI(this->GetToUri());
    }
    else if(this->mUAType == SIPSDK_UAS)
    {   
        ISipMessage* inviteMsg = this->mFirstCallMsg;

        ContactList contactList;
        if(inviteMsg)
            inviteMsg->GetContactList(contactList);

        // use the first contact as request URI, consider multiple contacts ??? 
        if(!contactList.empty())
            ack->SetRequestURI(contactList.front()->GetContact());
        else
            ack->SetRequestURI(this->GetFromUri());
    }
    else
        SIPSDK_ERRTRACE("SipCallImpl::CreateKeepAliveACK, unknow UA type: " << this->mUAType);
    
    return ack;
}

void SipCallImpl::SetCommonFields(ISipMessage * request)
{
    SIPSDK_ASSERT_RETURN_VOID(request);
    
    // Call-ID
    request->SetCallID(this->GetCallID());
    
    string localAddr;
    switch(mUAType)
    {
    case SIPSDK_UAC:
        localAddr = this->GetFromUri();
        break;
        
    case SIPSDK_UAS:
        localAddr = this->GetToUri();
        break;
    }
    
    // Add Via
    StrContainer viaList;
    request->GetViaList(viaList);
    
    if(viaList.empty())
    {
        stringstream via;
        via << "SIP/2.0/" << SipProviderImpl::instance()->GetTransportName();

        try
        {
            NameAddr na(localAddr.c_str());
            Uri & uri = na.uri();

            via << " " << uri.host().c_str();

            if(uri.port() > 0)
                via<< ":" << uri.port();
                
            viaList.push_back(via.str());
            request->SetViaList(viaList);
        
        } catch (std::exception & e) {
            SIPSDK_ERRTRACE("Uri parse failed, uri: " << localAddr);
        }
    }
    
    // Add Contact
    ContactList contactList;
    request->GetContactList(contactList);
   
    if(contactList.empty())
    {
        try
        {
            NameAddr na(localAddr.c_str());
            Uri & uri = na.uri();
            string uriStr(uri.toString().c_str());
            
            SipContactImpl::AddNewContact(contactList, uriStr, na.displayName().c_str());
        } catch (std::exception & e) {
            SIPSDK_ERRTRACE("Uri parse failed, uri: " << localAddr);
        }
        
        request->SetContactList(contactList);
        DestroyContactList(contactList);
    }
}

void SipCallImpl::SetSessionTimerParams(ISipMessage * request)
{
    SIPSDK_ASSERT_RETURN_VOID(request);

    // The following is for Session Timer special settings in SIP message
    // Ricky, 2011-11-30 16:57:08
    const SessionTimerParam& sessionTimerCfg = mSipCallMgr->GetSessionTimer();
    SDKSipMessage * msg = static_cast<SDKSipMessage*>(request);
    {
        if(sessionTimerCfg.sessExpires > 0 )
            msg->SetSessionExpires(sessionTimerCfg.sessExpires);
        
        if(sessionTimerCfg.minSessExpires > 0 )
            msg->SetMinSE(sessionTimerCfg.minSessExpires);
    }
    
    if(sessionTimerCfg.enabled)
    {
        msg->SetRefresher(this->mUAType);
        
        StrContainer supportedList;
        supportedList.push_back("timer");
        msg->SetSupportedList(supportedList);
    }
}

bool SipCallImpl::IsKeepAliveResponse(uint32 cseqNum)
{
    Lock lock(mSentKeepAliveCSeqNumLock);

    set<uint32>::iterator iter = mSentKeepAliveCSeqNum.find(cseqNum);
    return iter != mSentKeepAliveCSeqNum.end();
}

void SipCallImpl::AddTimerCSeq(uint32 cseqNum)
{
    Lock lock(mSentKeepAliveCSeqNumLock);
    mSentKeepAliveCSeqNum.insert(cseqNum);
}

void SipCallImpl::RemoveTimerCSeq(uint32 cseqNum)
{
    Lock lock(mSentKeepAliveCSeqNumLock);
    mSentKeepAliveCSeqNum.erase(cseqNum);
}

ISipCall * ISipCall::CreateSipCall(CStrRef sipFrom, CStrRef sipTo)
{
    ISipCall* call = SipCallMgr::GetInstance()->CreateSipCall(sipFrom, sipTo); 

    SIPSDK_INFOTRACE("SipCallImpl::CreateSipCall, call: " << call);
    return call;
}

