#ifndef _SIP_SDK_MESSAGE_IMPL_H__
#define _SIP_SDK_MESSAGE_IMPL_H__

#include "sipsdk.h"
#include "SipUtil.h"

class SDKSipMessage;

class SipUriUpdateListener
{
public:
    virtual void UpdateUriHandler() = 0;
};

class SipUriImpl : public ISipUri, public CReferenceControl
{
public:
    SipUriImpl(SipUriUpdateListener* listener);
    SipUriImpl(SipUriUpdateListener* listener, const Uri& uri);
    virtual ~SipUriImpl();
    
    virtual void        AddRef();
    virtual void        ReleaseRef();

    virtual string      GetScheme();
    virtual void        SetScheme(string scheme);

    virtual string      GetUser();
    virtual void        SetUser(string user);
    
    virtual string      GetPassword();
    virtual void        SetPassword(string pass);
    
    virtual string      GetHost();
    virtual void        SetHost(string host);

    virtual int         GetPort();
    virtual void        SetPort(int port);
    
    // to test if URI has a param, name including known & unknown parameters
    virtual bool        HasParam(string name);
    
    //"user", "transport", "maddr", "comp", "method"
    virtual string      GetDataParam(string name);
    virtual void        SetDataParam(string name, string value);
 
    
    virtual string      GetUnknownParam(string name);
    virtual void        SetUnknownParam(string name, string value);
    
    void                SetUri(const Uri & uri);
    const Uri &         GetUri() const;

    string              toString();
    bool                fromString(CStrRef str);

    void                NotifyListener();

private:
    Uri                     mUri;
    SipUriUpdateListener*   mUpdateListener;
};

class SipFromImpl : public ISipFrom,
                         public CReferenceControl,
                         public SipUriUpdateListener
{
public:
    SipFromImpl();
    SipFromImpl(CStrRef content);
    virtual ~SipFromImpl();

    // inherit from CReferenceControl
	virtual void    AddRef();
	virtual void    ReleaseRef();
    
    // copy URI & tag
    virtual void    Copy(ISipFrom * r);
 
    
    // inherit from SipUriUpdateListener
    virtual void    UpdateUriHandler();
    
    virtual CStrRef GetTag() const;
    virtual void    SetTag(string tag);
    
    virtual CStrRef GetFrom() const;
    virtual void    SetFrom(string from);

    virtual ISipUri* GetFromUri() const;

    virtual CStrRef toString() const;
    
    void            Encode();
    void            Decode();

    void            ConvertStr2Uri();
    void            ConvertUri2Str();
    
private:
    string      mContent;
    string      mTag;
    string      mFrom;
    SipUriImpl* mFromUri;
};

class SipToImpl : public ISipTo,
                      public CReferenceControl,
                      public SipUriUpdateListener
{
public:
    SipToImpl();
    SipToImpl(CStrRef content);
    virtual ~SipToImpl();

    // inherit from CReferenceControl
	virtual void    AddRef();
	virtual void    ReleaseRef();
	
	// copy URI & tag
    virtual void    Copy(ISipTo * r);

    // inherit from SipUriUpdateListener
    virtual void    UpdateUriHandler();
    
    virtual CStrRef GetTag() const;
    virtual void    SetTag(string tag);
    
    virtual CStrRef GetTo() const;
    virtual void    SetTo(string to);

    virtual ISipUri* GetToUri() const;

    virtual CStrRef toString() const;
    
    void            Encode();
    void            Decode();

    void            ConvertStr2Uri();
    void            ConvertUri2Str();
    
private:
    string      mContent;
    string      mTag;
    string      mTo;
    SipUriImpl* mToUri;
};

class SipContactImpl : public ISipContact,
                       public CReferenceControl,
                       public SipUriUpdateListener
{
public:
    SipContactImpl();
    virtual ~SipContactImpl();

    // inherit from CReferenceControl
	virtual void    AddRef();
	virtual void    ReleaseRef();

    // inherit from SipUriUpdateListener
    virtual void    UpdateUriHandler();
    
    virtual string   GetContact();
    virtual void     SetContact(string uri);
    virtual ISipUri* GetContactUri() const;
    
    virtual CStrRef     GetDisplayName() const;
    virtual void        SetDisplayName(CStrRef displayName);
    
    // Contact parameters "q" and "expires".
    virtual bool    HasQValue() const;
    virtual float   GetQValue() const;
    virtual void    SetQValue(float p_q);
    
    virtual bool    HasExpires() const;
    virtual int     GetExpires() const;
    virtual void    SetExpires(int p_expires);
    
    // return sip from representation string, "<sip:alice@atlanta.com>;expires=3600"
    virtual bool    fromString(CStrRef buf);
    virtual string  toString() ;
            void    toNameAddr(NameAddr & na);
    
    static void     AddNewContact(
                        ContactList & list,
                        CStrRef uri,
                        LPCSTR  display_name = "",
                        bool    has_expires  = false,
                        int     p_expires    = 0,
                        bool    has_q        = false,
                        float   p_q          = 1.0);

private:
    string       mDisplayName;
    bool         mHasParamQ;
    bool         mHasParamExpires;
    float        mParamQ;
    int          mParamExpires;
    
    SipUriImpl * mContactUri;
};

class SipCSeqImpl : public ISipCSeq,
                    public CReferenceControl
{
public:
    SipCSeqImpl();
    SipCSeqImpl(CStrRef cseq);
    virtual ~SipCSeqImpl();

    // inherit from CReferenceControl
	virtual void        AddRef();
	virtual void        ReleaseRef();
    
    virtual SipMethod   GetMethod() const;
    virtual void        SetMethod(SipMethod method);
    
    virtual int         GetNumber() const;
    virtual void        SetNumber(int num);

    virtual CStrRef     toString() const;
    
    void                Encode();
    void                Decode();
    
private:
    string      mCSeq;
    SipMethod   mMethod;
    int         mNum;
};

class SDKSipMessage : public ISipMessage,
                       public CReferenceControl
{
public:
    SDKSipMessage();
    virtual ~SDKSipMessage();

    // inherit from CReferenceControl
	virtual void                    AddRef();
	virtual void                    ReleaseRef();

    // inherit from ISipMessage
    // like "Also: sip:303@10.0.0.15"
    virtual string                  GetAlso() const;
    virtual void                    SetAlso(string value);
    
    virtual string                  GetRefer() const;
    virtual void                    SetRefer(string value);
   
    virtual CStrRef                 GetCallID() const;
    virtual void                    SetCallID(string callId);

    virtual void                    GetContactList(ContactList & result) const;
    virtual void                    SetContactList(ContactList & contactList);

    virtual void                    GetSupportedList(StrContainer & result) const;
    virtual void                    SetSupportedList(const StrContainer & contactList);
    virtual bool                    IsContainSupportedTag(string tag);

    virtual ISipCSeq*               GetCSeq() const;    
    virtual ISipFrom *              GetFrom() const;
    virtual ISipTo *                GetTo() const;

    virtual void                    SetCSeq(ISipCSeq * cseq);
    virtual void                    SetFrom(ISipFrom * from);
    virtual void                    SetTo(ISipTo * to);
    
    virtual void                    GetViaList(StrContainer & result) const;
    virtual void                    SetViaList(const StrContainer & viaList);

    // Added by Sam 08/16/2011 for KPML in CMS
    virtual void                    GetAllowEventList(StrContainer & result) const;
    virtual void                    SetAllowEventList(const StrContainer & allowEventList);
    
    // Ricky,  2011-10-20 11:45:50
    virtual void                    GetRequireList(StrContainer & result) const;
    virtual void                    SetRequireList(const StrContainer & requires);

    // Added by Sam 08/30/2011 for KPML in CMS
    virtual CStrRef                 GetEvent() const;
    virtual void                    SetEvent(string event);
    virtual void                    GetAcceptList(StrContainer & result) const;
    virtual void                    SetAcceptList(const StrContainer & acceptList);
    
    // Added by Sam for session timer
    virtual uint32                  GetSessionExpires() const;
    virtual void                    SetSessionExpires(uint32 sessionExpires);
    virtual uint32                  GetMinSE() const;
    virtual void                    SetMinSE(uint32 minSE);
    void                            SetRefresher(SIPSDK_UA_Type t);
    SIPSDK_UA_Type                  GetRefresher() const;
    // end add
    
    // get the session description
    virtual ISessionDescription *   GetSessionDescription() const;
    virtual void                    SetSessionDescription(ISessionDescription * sd);

    // get sip message body content, like msml
    virtual CStrRef                 GetBodyContent() const;
    virtual void                    SetBodyContent(string content);
    
    // get sip message content type
    virtual CStrRef                 GetContentType() const;
    virtual void                    SetContentType(string contentType);
    
    virtual SipMethod               GetRequstMethod() const;
    virtual void                    SetRequstMethod(SipMethod method);

    virtual CStrRef                 GetRequestURI() const;
    virtual void                    SetRequestURI(string requestURI);

    virtual int                     GetResponseStatusCode() const;
    virtual void                    SetResponseStatusCode(int code);
    
    virtual CStrRef                 GetResponseReason() const;
    virtual void                    SetResponseReason(string reason);

    virtual string                  toString() const;
    // inherit from ISipMessage

    virtual bool                    IsSipRequest() const;
    virtual void                    SetIsSipRequest(bool flag);

    virtual bool                    IsContainSdp() const;
    // <call-id, from tag, to tag> identify a unique dialog
    //CStrRef                         GetDialogID();
    string                          GetLocalTag();
    string                          GetRemoteTag();

private:
    string                          mAlso;
    string                          mRefer;
    string                          mCallID;
    ContactList                     mContactList;
    ISipCSeq*                       mCSeq;
    ISipFrom *                      mFrom;
    ISipTo *                        mTo;
    StrContainer                    mViaList;
    StrContainer                    mAllowEventList;
    StrContainer                    mRequireList;
    string                          mEvent;
    StrContainer                    mAcceptList;
    uint32                          mSessionExpires;
    uint32                          mMinSE;
    StrContainer                    mSupportedList;
    ISessionDescription*            mSessionDescription;
    string                          mBodyContent;
    string                          mContentType;
    bool                            mIsRequest;
    //string                          mDialogID;
    SipMethod                       mRequestMethod;
    string                          mRequestURI;
    int                             mResponseCode;
    string                          mResponseReason;
    SIPSDK_UA_Type                  mRefresher;
};


#endif
