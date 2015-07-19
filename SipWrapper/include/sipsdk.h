/**
*   Sip SDK interfaces
*   CSipSDK.h
*
*   These interfaces is defined for general perpose, we have to implement these
*   interfaces as codes in src folder, this is OOP code style.
*/

#ifndef _SIP_SDK_INTERFACE_H__
#define _SIP_SDK_INTERFACE_H__

#include "sipdef.h"

/**
*   Basically, ISipUri/ISipMessage/ISipCall/ISipProvider/ISipLogger is the
*   interface we explore. Just the class bellow who has no space before.
*/
class ISipUri;
class ISipMessage;
    class ISipFrom;
    class ISipTo;
    class ISipContact;
    class ISipCSeq;
    
    class ISessionDescription;
        class ISDOrigin;
        class ISDConnection;
        class ISDAttribute;
        class ISDTimeDescription;
            class ISDRepeatTime;

    class ISDMediaDescription;
        //class ISDMedia;
        class ISDCodec;
        class ISDAttribute;
        class ISDConnection;
        class ISDCrypto;
            class ISDCryptoParam;

class ISipCallSink;
class ISipCall;

class ISipProviderSink;
class ISipProvider;

class ISipLoggerSink;


/**
*   SDP is the origin packet attached in SIP packet.
*   ISDOrigin identifies SDP "o=" field, you would access it in 
*   "ISessionDescription" class.
*/
class ISDOrigin
{
public:
    virtual ~ISDOrigin() {};

    // reference control
    virtual void    AddRef() = 0;
    virtual void    ReleaseRef() = 0;

    // get/set owner(creator)
    virtual CStrRef GetUserName() const = 0;
    virtual void    SetUserName(string userName) = 0;

    // get/set session ID
    virtual int64   GetSessionID() const = 0;
    virtual void    SetSessionID(int64 sessionID) = 0;

    // get/set session version
    virtual int64   GetSessionVer() const = 0;
    virtual void    SetSessionVer(int64 sessionVer) = 0;

    // get/set network type, "IN" would been returned 
    virtual CStrRef GetNetType() const = 0;
    virtual void    SetNetType(string netType) = 0;

    // get/set address type, "IP4" or "IP6" would been returned
    virtual CStrRef GetAddressType() const = 0;
    virtual void    SetAddressType(string addressType) = 0;

    // get/set address
    virtual CStrRef GetAddress() const = 0;
    virtual void    SetAddress(string address) = 0;
};


/**
*   ISDRepeatTime identifies SDP "r=" field, you would access it
*   in "ISDTimeDescription" class
*/
class ISDRepeatTime
{
public:
    static ISDRepeatTime* Create(); 
    virtual ~ISDRepeatTime() {}

    // get/set repeat interval
    virtual int64   GetRepeatInterval() const = 0;
    virtual void    SetRepeatInterval(int64 interval) = 0;

    // get/set active duration
    virtual int64   GetActiveDuration() const = 0;
    virtual void    SetActiveDuration(int64 duration) = 0;

    // get/set offset
    virtual vector<int> GetOffset() const = 0;
    virtual void        SetOffset(const vector<int>& offset) = 0;
};


/**
*   ISDRepeatTime identifies SDP "t=" field, you would access it
*   in "ISessionDescription" class
*/
class ISDTimeDescription
{
public:
    typedef vector<ISDRepeatTime*> RTContainer;
    
public:
    static ISDTimeDescription* Create();
    virtual ~ISDTimeDescription() {}

    virtual int64   GetStartTime() const = 0;
    virtual void    SetStartTime(int64 startTime) = 0;
    
    virtual int64   GetStopTime() const = 0;
    virtual void    SetStopTime(int64 stopTime) = 0;
    
    virtual RTContainer& GetRepeatTimeList() = 0;
};

/**
*   ISDConnection identifies SDP "c=" field, there are 2 kinds of
*   connection, access session level connection in "ISessionDescription" 
*   class, access media level connection in "ISDMediaDescription"
*   
*   The "const" at the end of function presents this function could not
*   modify any private member variable. As C++ use copy method to return
*   CStrRef, so define "const" in return value is useless.
*/
class ISDConnection
{
public:
    static ISDConnection* Create();
    virtual ~ISDConnection() {}

    // reference control
    virtual void    AddRef() = 0;
    virtual void    ReleaseRef() = 0;

    // network type, the value should be "IN"
    virtual CStrRef GetNetType() const = 0;
    virtual void    SetNetType(string netType) = 0;

    // address type, default value is "V4"
    virtual CStrRef GetAddressType() const = 0;
    virtual void    SetAddressType(string addressType) = 0;

    // address
    virtual CStrRef GetAddress() const = 0;
    virtual void    SetAddress(string address) = 0;
};

/**
*   ISDAttribute identifies SDP "a=" field, there are 2 kinds of
*   attribute, access session level attribute in "ISessionDescription" 
*   class, access media level attribute in "ISDMediaDescription"
*/
class ISDAttribute
{
public:
    virtual ~ISDAttribute() {}

    // reference control
    virtual void            AddRef() = 0;
    virtual void            ReleaseRef() = 0;

    // check key whether exists in attribute, return true if exist
    virtual bool            IsKeyExist(CStrRef key) const = 0;

    // get all values by key
    virtual void            GetValues(CStrRef key, StrContainer& result) const = 0;

    // add a [key, value] pair into attribute
    virtual void            AddAttribute(CStrRef key, CStrRef value) = 0;

    // clear all values of key in attribute
    virtual void            ClearAttribute(CStrRef key) = 0;
};

/*
*   Media codec, every codec requires codec name, media rate and
*   payload type. This presents "m=" field.
*/
class ISDCodec
{
public:
    static ISDCodec* Create();

    virtual ~ISDCodec() {}

    // get codec name, return "telephone-event" of "a=rtpmap:101 telephone-event/8000"
    virtual CStrRef GetName() const = 0;
    virtual void    SetName(string name) = 0;

    // get codec rate, return "8000" of "a=rtpmap:101 telephone-event/8000"
    virtual uint32  GetRate() const = 0;
    virtual void    SetRate(uint32 rate) = 0;

    // get codec payload type, return "101" of "a=rtpmap:101 telephone-event/8000"
    virtual int     GetPayloadType() const = 0;
    virtual void    SetPayloadType(int payloadType) = 0;

    // get codec parameters
    // this interface would return "101 0-16" refer to following
    //      a=rtpmap:101 telephone-event/8000
    //      a=fmtp:101 0-16
    virtual CStrRef GetParameters() const = 0;
    virtual void    SetParameters(string parameters) = 0;
};

/*
*   crypto line sample: 
*   a=crypto:1 AES_CM_128_HMAC_SHA1_32 inline:d0RmdmcmVCspeEc3QGZiNWpVLFJhQX1cfHAwJSoj|2^20|1:32
*/
class ISDCrypto
{
public:
    static ISDCrypto* Create(); 
    virtual ~ISDCrypto() {}

    virtual uint32  GetTag() const = 0;
    virtual void    SetTag(uint32 tag) = 0;

    virtual CStrRef GetCryptoSuiteType() const = 0;
    virtual void    SetCryptoSuiteType(string type) = 0;

    virtual vector<ISDCryptoParam*>& GetAllParams() = 0;
};

/**
*   Used in SDP.
*/
class ISDCryptoParam
{
public:
    static ISDCryptoParam* Create(); 
    virtual ~ISDCryptoParam() {}
    
    virtual CStrRef GetBase64Key() const = 0;
    virtual void    SetBase64Key(string base64Key) = 0;

    virtual CStrRef GetMasterKey() const = 0;
    virtual void    SetMasterKey(string masterKey) = 0;

    virtual CStrRef GetMasterSalt() const = 0;
    virtual void    SetMasterSalt(string masterSalt) = 0;

    virtual uint32  GetLifeTime() const = 0;
    virtual void    SetLifeTime(uint32 lifeTime) = 0;

    virtual uint32  GetMasterKeyIndex() const = 0;
    virtual void    SetMasterKeyIndex(uint32 index) = 0;

    virtual uint32  GetMasterKeyIndexLen() const = 0;
    virtual void    SetMasterKeyIndexLen(uint32 len) = 0;
};

/**
*   SDP media information, used in SDP.
*/
class ISDMediaDescription
{
public:
    static ISDMediaDescription*     Create();
 
    virtual ~ISDMediaDescription() {}

    // get media type, return "audio" of "m=audio 49152 RTP/AVP 3 97 98 8 0 101"
    virtual CStrRef                 GetMediaType() const = 0;
    virtual void                    SetMediaType(string mediaType) = 0;

    // get media port, return "49152" of "m=audio 49152 RTP/AVP 3 97 98 8 0 101"
    virtual int                     GetMediaPort() const = 0;
    virtual void                    SetMediaPort(int mediaPort) = 0;

    // get media protocol, return "RTP/AVP" of "m=audio 49152 RTP/AVP 3 97 98 8 0 101"
    virtual CStrRef                 GetProtocol() const = 0;
    virtual void                    SetProtocol(string protocol) = 0;

    // get media payload type list, return "3 97 98 8 0 101" of "m=audio 49152 RTP/AVP 3 97 98 8 0 101"
    virtual vector<int>&            GetPayLoadTypeList() = 0;

    // get media information
    virtual CStrRef                 GetInfo() const = 0;
    virtual void                    SetInfo(string info) = 0;

    // get media connections
    virtual vector<ISDConnection*>& GetConnectionList() = 0;

    // get media attribute object, you would access attribute key & value by this object
    virtual ISDAttribute*           GetAttribute() const = 0;

    // get all codecs of this media
    virtual vector<ISDCodec*>&      GetAllCodecs() = 0;

    // get all codecs of this media
    virtual vector<ISDCrypto*>&     GetAllCryptos() = 0;
    // clear all codecs of this media
    virtual void                    ClearAllCryptos() = 0;
};

/**
*   class ISdpInfo indicates a sdp information, application can get sdp information 
*   or modify sdp information through this class
*
*   SDK would implement this interface class
*/
class ISessionDescription
{
public:
    typedef vector<ISDTimeDescription*>             TimeDescrContainer;
    typedef vector<ISDTimeDescription*>::iterator   TimeDescrContainerIter;

    typedef vector<ISDMediaDescription*>            MediaDescrContainer;
    typedef MediaDescrContainer::iterator           MediaDescrContainerIter;
    
public:
    // parse text and create a ISessionDescription object automatically
    // you are required to release the returned object after done.
    static ISessionDescription * Create(string * sdp = NULL);

    virtual ~ISessionDescription() {}

    // reference control
    virtual void                 AddRef() = 0;
    virtual void                 ReleaseRef() = 0;

    // negotiate sdp, return a final sdp, return NULL if negotiate failed
    virtual ISessionDescription* Negotiate(ISessionDescription* remoteSdp) = 0;

    virtual ISessionDescription* Clone() = 0;

    // get sdp raw content text
    virtual CStrRef              GetRawContent() const = 0;

    // get sdp version
    virtual int                  GetVersion() const = 0;
    virtual void                 SetVersion(int version) = 0;

    // get session name
    virtual CStrRef              GetSessionName() const = 0;
    virtual void                 SetSessionName(string sessionName) = 0;

    // get sdp information
    virtual CStrRef              GetInfo() const = 0;
    virtual void                 SetInfo(string info) = 0;

    // get sdp uri
    virtual CStrRef              GetURI() const = 0;
    virtual void                 SetURI(string uri) = 0;

    // o=  (originator and session identifier)
    virtual ISDOrigin*           GetOrigin() const = 0;

    //  c=* (connection information -- not required if included in all media)
    virtual ISDConnection*       GetConnection() const = 0;

    // a=* (zero or more session attribute lines)
    virtual ISDAttribute*        GetAttribute() const = 0;

    // get sdp emails
    virtual StrContainer&        GetEmailList() = 0;

    // get sdp phones
    virtual StrContainer&        GetPhoneList() = 0;

    // return sdp representation string text
    virtual string               toString() = 0;

    // get sdp time description list
    virtual TimeDescrContainer&  GetTimeDescriptionList() = 0;

    // get sdp media description list
    virtual MediaDescrContainer& GetMediaDescriptionList() = 0;
};

/**
*   SIP uri, used in SIP request.
*/
class ISipUri
{
public:
    static ISipUri * newInstance();
    
    virtual ~ISipUri() {}
    
    // reference control
    virtual void        AddRef() = 0;
    virtual void        ReleaseRef() = 0;

    virtual string      GetScheme() = 0;
    virtual void        SetScheme(string scheme) = 0;
    
    virtual string      GetUser() = 0;
    virtual void        SetUser(string user) = 0;

    virtual string      GetPassword() = 0;
    virtual void        SetPassword(string pass) = 0;
    
    virtual string      GetHost() = 0;
    virtual void        SetHost(string host) = 0;

    virtual int         GetPort() = 0;
    virtual void        SetPort(int port) = 0;

    // To test if URI has a named param, like "user", "AttendeeID"
    virtual bool        HasParam(string name) = 0;

    //"user", "transport", "maddr", "comp", "method", "ttl"
    virtual string      GetDataParam(string name) = 0;
    virtual void        SetDataParam(string name, string value) = 0;
    
    
    // like "AttendeeID" = "12345"
    // name couldn't be system defined keywords like "user", "transport", "comp", "ttl", "method", "maddr", "lr"
    virtual string      GetUnknownParam(string name) = 0;
    virtual void        SetUnknownParam(string name, string value) = 0;
    
    // buf is like: "sips:alice:pass@example.com;user=XXX;transport=ZZZ;AttendeeID=YYYY"
    virtual bool        fromString(CStrRef buf) = 0;
    virtual string      toString() = 0;
};

/**
*   SIP from field used in request.
*/
class ISipFrom
{
public:
    virtual ~ISipFrom() {}

    // reference control
    virtual void        AddRef() = 0;
    virtual void        ReleaseRef() = 0;
    
    // copy URI & tag
    virtual void        Copy(ISipFrom * r) = 0;
    
    // sip from tag
    virtual CStrRef     GetTag() const = 0;
    virtual void        SetTag(string tag) = 0;

    // sip from URI, URI format "sip:msml@10.224.54.27:5060"
    virtual CStrRef     GetFrom() const = 0;
    virtual void        SetFrom(string from) = 0;

    // get from URI, would read/write user/host/port through URI object
    // SHOULD NOT use this function if you invoke GetFrom/SetFrom functions above 
    virtual ISipUri*    GetFromUri() const = 0;

    // return sip from representation string, "<sip:6434@10.224.54.213>;tag=b8f2ef1659d"
    virtual CStrRef     toString() const = 0;
};

class ISipTo
{
public:
    virtual ~ISipTo() {}

    // reference control
    virtual void        AddRef() = 0;
    virtual void        ReleaseRef() = 0;

    // copy URI & tag
    virtual void        Copy(ISipTo * r) = 0;

    // sip to tag
    virtual CStrRef     GetTag() const = 0;
    virtual void        SetTag(string tag) = 0;

    // sip to URI, URI format "sip:msml@10.224.54.27:5060"
    virtual CStrRef     GetTo() const = 0;
    virtual void        SetTo(string to) = 0;

    // get to URI, would read/write user/host/port through URI object
    // SHOULD NOT use this function if you invoke GetTo/SetTo functions above 
    virtual ISipUri*    GetToUri() const = 0;

    virtual CStrRef     toString() const = 0;
};

class ISipContact
{
public:
    // Example:
    /* ISipContact * instance = ISipContact::newInstance();
       instance->AddRef();
       instance->...
       instance->ReleaseRef();
    */
    static ISipContact * newInstance();
        
    virtual ~ISipContact() {}

    // reference control
    virtual void        AddRef() = 0;
    virtual void        ReleaseRef() = 0;
    
    // sip to URI, URI format "sip:alice@pc33.atlanta.com"
    virtual string      GetContact() = 0;
    virtual void        SetContact(string uri) = 0;
    
    virtual CStrRef     GetDisplayName() const = 0;
    virtual void        SetDisplayName(CStrRef displayName) = 0;
    
    // get to URI, would read/write user/host/port through URI object
    // SHOULD NOT use this function if you invoke GetContact/SetContact functions above 
    virtual ISipUri*    GetContactUri() const = 0;
    
    // Contact parameters "q" and "expires".
    virtual bool        HasQValue() const = 0;
    virtual float       GetQValue() const = 0;
    virtual void        SetQValue(float p_q) = 0;

    virtual bool        HasExpires() const = 0;
    virtual int         GetExpires() const = 0;
    virtual void        SetExpires(int p_expires) = 0;
    
    // return sip from representation string, "<sip:alice@atlanta.com>;expires=3600"
    virtual bool        fromString(CStrRef buf) = 0;
    virtual string      toString()  = 0;
};

class ISipCSeq
{
public:
    virtual ~ISipCSeq() {}

    // reference control
    virtual void        AddRef() = 0;
    virtual void        ReleaseRef() = 0;

    // get sip method of CSeq
    virtual SipMethod   GetMethod() const = 0;
    virtual void        SetMethod(SipMethod method) = 0;

    // get sequence number
    virtual int         GetNumber() const = 0;
    virtual void        SetNumber(int num) = 0;

    virtual CStrRef     toString() const = 0;
};

typedef vector<ISipContact *>  ContactList;
typedef ContactList::iterator  ContactIter;

/**
// class ISipMessage indicates a sip message, this class would been used in following:
// 1) in ISipCallSink, the SDK transfer a object of this class to up application to access Sip msg
//    such as: get cseq data, sdp information &etc.
// 2) in ISipCall, application transfer a object of this class to SDK, SDK would generate a sip message 
//    and sent it out.
// 
// SDK would implement this interface class
*/
class ISipMessage
{
public:
    virtual ~ISipMessage() {}

    // reference control
    virtual void            AddRef() = 0;
    virtual void            ReleaseRef() = 0;

    // like "Also: sip:303@10.0.0.15"
    /*
        ISipUri * uri = ISipUri::newInstance();
        uri->AddRef();

         string also = msg->GetAlso();
         if(!also.empty())
         {
            if(uri->fromString(s))
                //...
         }
     */
    virtual string          GetAlso() const = 0;
    virtual void            SetAlso(string value) = 0;

    virtual string          GetRefer() const = 0;
    virtual void            SetRefer(string value) = 0;

    // get Call-ID header field value
    virtual CStrRef         GetCallID() const = 0;
    virtual void            SetCallID(string callID) = 0;

    // get Contact header fields value
    // we define Contact as a string format like "sip:192.168.2.17:5103"
    virtual void            GetContactList(ContactList  & result) const = 0;
    virtual void            SetContactList(ContactList  & contactList) = 0;

    // get object pointer of CSeq
    virtual ISipCSeq *      GetCSeq() const = 0;

    // get object pointer of Sip From
    virtual ISipFrom *      GetFrom() const = 0;

    // get object pointer of Sip To
    virtual ISipTo *        GetTo() const = 0;

    // get Via header fields
    // we define Via as a string format like "SIP/2.0/UDP 10.224.54.27:5299"
    virtual void            GetViaList(StrContainer & result) const = 0;
    virtual void            SetViaList(const StrContainer & viaList) = 0;

    // get Allow-Event fields
    virtual void            GetAllowEventList(StrContainer & result) const = 0;
    virtual void            SetAllowEventList(const StrContainer & allowEventList) = 0;
    
    // get Require fields
    virtual void            GetRequireList(StrContainer & result) const = 0;
    virtual void            SetRequireList(const StrContainer & requires) = 0;

    // get Event fields
    virtual CStrRef         GetEvent() const = 0;
    virtual void            SetEvent(string event) = 0;

    // get Accept fields
    virtual void            GetAcceptList(StrContainer & result) const = 0;
    virtual void            SetAcceptList(const StrContainer & acceptList) = 0;

    // get Supported fields
    virtual void            GetSupportedList(StrContainer & result) const = 0;
    virtual void            SetSupportedList(const StrContainer & supportedList) = 0;

    // get the session description
    virtual ISessionDescription *   GetSessionDescription() const = 0;
    virtual void                    SetSessionDescription(ISessionDescription*) = 0;

    // get sip message body content, like msml
    virtual CStrRef         GetBodyContent() const = 0;
    virtual void            SetBodyContent(string content) = 0;

    // get sip message content type
    virtual CStrRef         GetContentType() const = 0;
    virtual void            SetContentType(string contentType) = 0;

    // return TRUE if message is a request
    virtual bool            IsSipRequest() const = 0;

    // return TRUE if message contains a sdp
    virtual bool            IsContainSdp() const = 0;

    // get request method
    virtual SipMethod       GetRequstMethod() const = 0;
    virtual void            SetRequstMethod(SipMethod method) = 0;

    // get request URI
    virtual CStrRef         GetRequestURI() const = 0;
    virtual void            SetRequestURI(string requestURI) = 0;

    // get response status code
    virtual int             GetResponseStatusCode() const = 0;
    virtual void            SetResponseStatusCode(int code) = 0;

    // get response reason
    virtual CStrRef         GetResponseReason() const = 0;
    virtual void            SetResponseReason(string reason) = 0;

    // get sip message presentation
    virtual string          toString() const = 0;
    
    static ISipMessage *    CreateSipRequest();
    //static ISipMessage *    CreateSipResponse();
    static ISipMessage *    CreateSipResponse(ISipMessage* request);
};

/**
*   class ISipCallSink provides multiple callback functions to application
*   when SDK accept incoming sip message, the SDK would notify application by calling callback functions
*/
class ISipCallSink
{
public:
    virtual ~ISipCallSink()  {}

    // user(application) should implement these 2 functions for reference control
    virtual void AddReference() = 0;
    virtual void ReleaseReference() = 0;

    // callback function for receive a INVITE sip request message from UA    
    virtual CmResult OnRecvInviteRqst(ISipCall * call, ISipMessage * request) = 0;

    // callback function for receive a ringing(100) sip response message from UA
    virtual CmResult OnRecvTrying(ISipCall * call, ISipMessage * request) = 0;
    
    // callback function for receive a ringing(180) sip response message from UA
    virtual CmResult OnRecvRinging(ISipCall * call, ISipMessage * request) = 0;
    
    // callback function for receive INVITE sip response message from UA, the response maybe 200 or not
    virtual CmResult OnRecvInviteResp(ISipCall * call, ISipMessage * response) = 0;

    // callback function for receive a ACK sip message from UA
    virtual CmResult OnRecvACKRqst(ISipCall * call, ISipMessage * request) = 0;

    // callback function for receive a BYE sip message from UA
    virtual CmResult OnRecvByeRqst(ISipCall * call, ISipMessage * request) = 0;

    // callback function for receive a CANCEL sip message from UA
    virtual CmResult OnRecvCancelRqst(ISipCall * call, ISipMessage * request) = 0;

    // callback function for receive a INFO sip message from UA
    virtual CmResult OnRecvInfoRqst(ISipCall * call, ISipMessage * request) = 0;

    // callback function for receive a OPTIONS sip message from UA
    virtual CmResult OnRecvOptionsRqst(ISipCall * call, ISipMessage * request) = 0;

    // callback function for receive a OPTIONS sip message from UA
    virtual CmResult OnRecvSubscribeRqst(ISipCall * call, ISipMessage * request) = 0;

    // callback function for receive a OPTIONS sip message from UA
    virtual CmResult OnRecvNotifyRqst(ISipCall * call, ISipMessage * request) = 0;

    // callback function for receive a sip response message from UA
    virtual CmResult OnRecvResp(ISipCall * call, ISipMessage* response) = 0;

    // callback function when session timeout
	virtual CmResult OnSessionTimeout() = 0;
};

/***
*   class ISipCall presents SIP session, provides application to send multiple sip message like Invite, Ringing.
*   As RFC3261 said, SIP session presents a session between UA. From this session, UA could get the state and process by this state.
*/
class ISipCall
{
public:
    virtual ~ISipCall() {};

    /*  Basically, add and release reference should be implement by an 
    *   interface, just like Java.
    */
	virtual void AddRef() = 0;
	virtual void ReleaseRef() = 0;

    /*  create a new sip call
    *   @param sipFromUri, URI format "sip:msml@10.224.54.27:5060"
    *   @param sipToUri, URI format "sip:msml@10.224.54.27:5060""
    */
    static ISipCall *       CreateSipCall(CStrRef sipFromUri, CStrRef sipToUri);
    
    //  set/get sip call sink
    virtual void            SetSipCallSink(ISipCallSink * sink) = 0;
    virtual ISipCallSink *  GetSipCallSink() const = 0;

    // send sip INVITE requset message to UA
    virtual CmResult Invite(ISipMessage * request) = 0;
    //virtual CmResult Invite(string & target) = 0;

    // send sip RINGING requset message to UA
    virtual CmResult Ringing(ISipMessage * response) = 0;

    // send sip OK(200) response message to UA
    virtual CmResult OK(ISipMessage * response) = 0;
    
    // send sip RESPONSE message to UA
    virtual CmResult Respond(ISipMessage * response) = 0;

    // send sip cancel request message to UA
    virtual CmResult Cancel(ISipMessage * request) = 0;

    // send sip ACK request message to UA
    virtual CmResult ACK(ISipMessage * request) = 0;
    
    // send sip BYE request message to UA
    virtual CmResult Bye(ISipMessage * request) = 0;

    // send sip OPTION request message to UA
    virtual CmResult Options(ISipMessage * request) = 0;

    // send sip INFO request message to UA
    virtual CmResult Info(ISipMessage * request) = 0;

    // send sip SUBSCRIBE request message to UA
    virtual CmResult Subscribe(ISipMessage * request) = 0;

    // send sip NOTIFY request message to UA
    virtual CmResult Notify(ISipMessage * request) = 0;

    // destroy the sip call
    virtual CmResult Destroy() = 0;

    // set call from URI, URI format "sip:msml@10.224.54.27:5060"
    virtual CmResult SetCallFromURI(string fromURI) = 0;

    // set call to URI, URI format "sip:msml@10.224.54.27:5060"
    virtual CmResult SetCallToURI(string toURI) = 0;
};

class StackStat
{
public:
    typedef enum 
    {
        TU_FIFO_SIZE,
        TRANSPORT_FIFO_SIZE_SUM,
        TRANSACTION_FIFO_SIZE,
        ACTIVE_TIMER_NUM,
        OPEN_TCP_CONNECTION_NUM,
        ACTIVE_CLIENT_TRANSACTION_NUM,
        ACTIVE_SERVER_TRANSACTION_NUM,
        PENDING_DNS_QUERY_NUM,

        SENT_REQUEST_NUM,
        SENT_RESPONSE_NUM,
        RETRANSMIT_REQUEST_NUM,
        RETRANSMIT_RESPONSE_NUM,
        RECEIVE_REQUEST_NUM,
        RECEIVER_RESPONSE_NUM
    } StatType;

    virtual ~StackStat() {};

    virtual void    AddRef() = 0;
	virtual void    ReleaseRef() = 0;
    
    virtual uint32  GetValue(StatType type) const = 0;
    virtual uint32  Get2xxInSum(SipMethod method) const = 0;
    virtual uint32  GetErrInSum(SipMethod method) const = 0;
    virtual uint32  Get2xxOutSum(SipMethod method) const = 0;
    virtual uint32  GetErrOutSum(SipMethod method) const = 0;

};
      
// User implemented
class ISipProviderSink
{
public:
	virtual void    AddReference() = 0;
	virtual void    ReleaseReference() = 0;

    // notify user that a new call comming
    virtual CmResult OnCallIndication(ISipCall * call) = 0;
};

typedef enum 
{
    STACK_LOG_NONE,
    STACK_LOG_ERROR,
    STACK_LOG_INFO,
    STACK_LOG_DETAIL
} StackLogLevel;

class ISipProvider
{
public:
    virtual ~ISipProvider() {};
    
    // set sip provider sink
    virtual void SetSipProviderSink(ISipProviderSink * sink) = 0;

    // set TLS certfication file path
    virtual void SetTlsCertPath(string certPath) = 0;

    // set TLS certfication domain name
    virtual void SetTlsDomain(string domainName) = 0;
    
    // add sip transport
    virtual bool AddTransport(
                    TransportProtocol protocol,
                    CStrRef           ipAddress = "",
                    int               port = 0,
                    AddressType       type = IPV4,
                    QosType           dscp = 0 ) = 0;
       
    virtual bool Start() = 0;
    virtual void Stop() = 0;

	// set session timer
	virtual void SetupSessionTimer(bool runing, int sessExpires, int minSessExpires) = 0;

    // get sip stack statistic
    virtual StackStat* GetStackStat() = 0;

    // set sip stack log path & log level
    virtual void SetStackLogPath(string path, string filePrefix = "", StackLogLevel level = STACK_LOG_INFO) = 0;
    
    // set sip SDK options
    virtual int SetOpt(SipOption opt, void * s, size_t n) = 0;
    
public:
    // start/stop sip provider instance
    static ISipProvider *   GetInstance();
    static void             DestroyInstance();
};

class ISipLoggerSink
{
public:
    virtual ~ISipLoggerSink() {};

	virtual void    AddReference() = 0;
	virtual void    ReleaseReference() = 0;
    
    virtual void    Detail(const char*) = 0;
    virtual void    Info(const char*) = 0;
    virtual void    Warning(const char*) = 0;
    virtual void    Error(const char*) = 0;
};

extern "C"
{
    void SetupSipSDKLoggerSink(ISipLoggerSink * sink);
    void RemoveSipSDKLoggerSink();
    void DestroyContactList(ContactList & list);
}

#endif
