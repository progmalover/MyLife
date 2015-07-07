#ifndef _SIP_SDK_CALL_IMPL_H__
#define _SIP_SDK_CALL_IMPL_H__

#include "sipsdk.h"
#include "SipMessageImpl.h"
#include "SipUtil.h"

typedef enum 
{
    CALL_STATE_START = 0,
    CALL_STATE_SENT_INVITE,
    CALL_STATE_RECEIVED_INVITE,
    CALL_STATE_RECEIVED_200,
    CALL_STATE_RECEIVED_302,
    CALL_STATE_SESSION_TIME_OUT,
    CALL_STATE_COMPLETE,
    CALL_STATE_SENT_BYE,
	CALL_STATE_SENT_BYE_RESPONSE,
    CALL_STATE_RECEIVED_BYE_RESPONSE,
    CALL_STATE_DESTROYING,
    CALL_STATE_UNKNOWN
} SipCallState;

class SipProviderImpl;
class SipCallMgr;

   
// Implement ISipCall, ISipCall indicates a sip peer-peer sip call leg
class SipCallImpl : public ISipCall,
                    public CReferenceControl
{
    enum { CALL_DESTROY_DELAY = 5 };
    
public:
    SipCallImpl(
        SipCallMgr *      callMgr, 
        CStrRef           sipFrom,
        CStrRef           sipTo,
        CStrRef           sipFromTag,
        CStrRef           sipToTag,
        CStrRef           sipCallID,
        ISipMessage*      firstCallMsg, 
        SIPSDK_UA_Type    type = SIPSDK_UAC);
    virtual ~SipCallImpl();
    
	virtual void AddRef();
	virtual void ReleaseRef();
    
    // set/get sip call sink
    virtual void SetSipCallSink(ISipCallSink * sink);
    virtual ISipCallSink * GetSipCallSink() const;

    // send sip invite requset message to UA
    virtual CmResult    Invite(ISipMessage * request);
    //CmResult Invite(string & target);

    // send sip ringing requset message to UA
    virtual CmResult    Ringing(ISipMessage* response);

    // send sip OK(200) response message to UA
    virtual CmResult    OK(ISipMessage* response);
    
    // send sip response message to UA
    virtual CmResult    Respond(ISipMessage* response);

    // send sip cancel request message to UA
    virtual CmResult    Cancel(ISipMessage * request);

    // send sip bye request message to UA
    virtual CmResult    ACK(ISipMessage * request);
    
    // send sip bye request message to UA
    virtual CmResult    Bye(ISipMessage * request);

    // send sip option request message to UA
    virtual CmResult    Options(ISipMessage * request);

    // send sip info request message to UA
    virtual CmResult    Info(ISipMessage * request);

    // send sip subscribe request message to UA
    virtual CmResult    Subscribe(ISipMessage * request);

    // send sip notify request message to UA
    virtual CmResult    Notify(ISipMessage * request);

    // destroy the sip call
    virtual CmResult    Destroy();

    // set call from URI
    virtual CmResult    SetCallFromURI(string fromURI);

    // set call to URI
    virtual CmResult    SetCallToURI(string toURI);

    // Destroy() only update call state to destroy
    bool                ShouldDelete() const;
    
    // set/get sip call state
    SipCallState        GetCallState() const;
    LPCSTR              GetCallStateStr() const;
    void                SetCallState(SipCallState state);
    

    void                SetFirstCallMsg(ISipMessage* callFirstMsg);
    ISipMessage*        GetFirstCallMsg() const;
        
    CStrRef             GetFromUri() const;
    CStrRef             GetToUri() const;

    void                SetFromTag(string fromTag);
    CStrRef             GetFromTag() const;
    void                GenFromTag();

    void                SetToTag(string toTag);
    CStrRef             GetToTag() const;
    void                GenToTag();

    void                SetLocalUri(string localUri);
    CStrRef             GetLocalUri() const;

    void                SetRemoteUri(string remoteUri);
    CStrRef             GetRemoteUri() const;

    void                SetLocalTag(string localTag);
    CStrRef             GetLocalTag() const;

    void                SetRemoteTag(string remoteTag);
    CStrRef             GetRemoteTag() const;
        
    void                SetCallID(string callID);
    CStrRef             GetCallID() const;

    void                SetRemoteCSeqNum(uint32 num);
    uint32              GetRemoteCSeqNum();
    uint32              GetRemoteNextCSeqNum();

    void                SetSessionTimer(bool localRefresher, bool farRefresher, uint32 sessionInterval);
    uint32              GetSessionInterval() const;

    bool                NeedSendKeepAlive();
    void                SendKeepAlive();
    bool                IsTimeOut();
    
    void                UpdateRecvKATimestamp();
    void                UpdateSendKATimestamp();
    
    bool                IsKeepAliveResponse(uint32 cseqNum);
    void                SetLastMethodUsedInRequest(SipMethod method);

    void                SetUAType(SIPSDK_UA_Type uaType);
    SIPSDK_UA_Type      GetUAType() const;
   
    // Convert SDK message to resip message, and call SipCallMgr to send resip message
    CmResult            SendMessage(ISipMessage * msg);
    
    // Process incoming messages
    CmResult             ProcessRequest(SDKSipMessage * request);
    CmResult            ProcessResponse(SDKSipMessage * response);
     
    
private:
    void                SetLocalCSeqNum(uint32 num);
    uint32              GetLocalCSeqNum();
    uint32              GetLocalNextCSeqNum();

    void                SetupRequest(ISipMessage *  request, SipMethod method);
    void                SetupResponse(ISipMessage * request, SipMethod method = SIP_METHOD_UNKNOWN);
    
    // Save local SDP from OK/Invite, for future re-Invite.
    void                SaveLocalSDP(ISessionDescription* sdp);

    // Send a request, if [request] is NULL, SDK setup a default 
    CmResult            SendRequest(SipMethod method, ISipMessage * request = NULL);
    
    ISipMessage*        CreateReInviteRequest();
    ISipMessage*        CreateKeepAliveACK(ISipMessage* response);
    CmResult            SendReInvite(ISipMessage * keepAlive);
    CmResult            SendKeepAliveACK(ISipMessage* response);
    
    // Process "invite" request sip message
    CmResult            ProcessInviteRequest(SDKSipMessage * msg);

    // Process "non-invite" request sip message includes: OPTIONS/REGISTER/MESSAGE/REFER/PUBLISH/SUBSCRIBE
    // These methods belong to "non-dialog call", would create a temporary call to process them, and
    // the temporary call would be destroyed immediately
    CmResult            ProcessNonDialogRequest(SDKSipMessage * msg);
    
    // Process "re-invite" request sip message
    CmResult            ProcessReInviteRequest(SDKSipMessage * msg);
    
    // Process "non-invite" request sip message includes: ACK, BYE, INFO
    // These methods relate a "dialog call", would look for a live call to problem them
    CmResult            ProcessNonInviteRequset(SDKSipMessage * msg);
    CmResult            Respond422(SDKSipMessage * rqst, uint32 minSE);
    CmResult            RespondError(SDKSipMessage * rqst, int code);
    
    // Process RESPONSE codes
    CmResult            ProcessResponse302(SDKSipMessage * response);
    CmResult            ProcessResponse422(SDKSipMessage * response);
    CmResult            ProcessResponse200(SDKSipMessage * response);

    // handle session timer
    CmResult            HandleSessionTimerNegotiation(SDKSipMessage * msg);
    void                AddTimerCSeq(uint32     seq);
    void                RemoveTimerCSeq(uint32  seq);
    
    // setup Via, Contact, Call-ID
    void                SetCommonFields(ISipMessage * request);
    void                SetSessionTimerParams(ISipMessage * request);
    
private:
    SipCallMgr *        mSipCallMgr;
    SipCallState        mCallState;
    ISipCallSink *      mSipCallSink;
    time_t              mCreateTime;
    int                 mFailureCode;

    SIPSDK_UA_Type      mUAType;
    string              mFromUri;
    string              mFromTag;
    string              mToUri;
    string              mToTag;
    string              mLocalUri;
    string              mLocalTag;
    string              mRemoteUri;
    string              mRemoteTag;
    string              mCallID;
    AtomicUInteger      mRemoteCSeqNum;
    AtomicUInteger      mLocalCSeqNum;
    
    SipMethod           mLastMethodUsedInRequest;
        
    uint32              mSessionInterval;   // seconds
    bool                mLocalRefresher;
    bool				mFarRefresher;
    time_t              mLastSendKATime;
    time_t              mLastRecvKATime;
    time_t              mDestroyTime;
    

    ISipMessage*        mFirstCallMsg;

    ISessionDescription*    mLocalSDP;
    ISessionDescription*    mLastNegotiationSdp;

    set<uint32>         mSentKeepAliveCSeqNum;
    Mutex               mSentKeepAliveCSeqNumLock;

    int                 mLastRespCSeq;
    int                 mLastRespCode;
};

#endif
