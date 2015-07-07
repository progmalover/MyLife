#ifndef _SIP_SDK_SESSION_DESCRIPTION_IMPL_H__
#define _SIP_SDK_SESSION_DESCRIPTION_IMPL_H__

#include "sipsdk.h"
#include "AttributeImpl.h"
#include "SipUtil.h"

class SDOriginImpl : public ISDOrigin,
                     public CReferenceControl
{
public:
    SDOriginImpl();
    virtual ~SDOriginImpl();
    
	virtual void    AddRef();
	virtual void    ReleaseRef();

    virtual CStrRef GetUserName() const;
    virtual void    SetUserName(string userName);
    
    virtual int64   GetSessionID() const;
    virtual void    SetSessionID(int64 sessionID);
    
    virtual int64   GetSessionVer() const;
    virtual void    SetSessionVer(int64 sessionVer);
    
    virtual CStrRef GetNetType() const;
    virtual void    SetNetType(string netType);
    
    virtual CStrRef GetAddressType() const;
    virtual void    SetAddressType(string addressType);
    
    virtual CStrRef GetAddress() const;
    virtual void    SetAddress(string address);
    
private:
    string  mUserName;
    int64   mSessionID;
    int64   mSessionVer;
    string  mNetType;
    string  mAddressType;
    string  mAddress;
};

class SDRepeatTimeImpl : public ISDRepeatTime
{
public:
    SDRepeatTimeImpl();
    virtual ~SDRepeatTimeImpl();
    
	//virtual void    AddRef();
	//virtual void    ReleaseRef();
	
    virtual int64   GetRepeatInterval() const;
    virtual void    SetRepeatInterval(int64 interval);

    virtual int64   GetActiveDuration() const;
    virtual void    SetActiveDuration(int64 duration);
    
    virtual vector<int> GetOffset() const;
    virtual void        SetOffset(const vector<int>& offset);

private:
    int64       mRepeatInterval;
    int64       mActiveDuration;
    vector<int> mOffset;
};

class SDTimeDescriptionImpl : public ISDTimeDescription
{
public:
    SDTimeDescriptionImpl();
    virtual ~SDTimeDescriptionImpl();
    
	//virtual void    AddRef();
	//virtual void    ReleaseRef();
	
    virtual int64   GetStartTime() const;
    virtual void    SetStartTime(int64 startTime);
    virtual int64   GetStopTime() const;
    virtual void    SetStopTime(int64 stopTime);    
    
    virtual RTContainer& GetRepeatTimeList();

private:
    int64   mStartTime;
    int64   mStopTime;
    ISDTimeDescription::RTContainer mRepeatTime;
};

class SessionDescriptionImpl : public ISessionDescription,
                               public CReferenceControl
{
public:
    SessionDescriptionImpl();
    virtual ~SessionDescriptionImpl();

    static SessionDescriptionImpl * Create(CStrRef rawContent);
    
	virtual void            AddRef();
	virtual void            ReleaseRef();

    virtual ISessionDescription* Negotiate(ISessionDescription* remoteSdp);
    virtual ISessionDescription* Clone() ;
    
    virtual CStrRef         GetRawContent() const;
    virtual void            SetRawContent(string content);
    
    virtual int             GetVersion() const;
    virtual void            SetVersion(int version);
    
    virtual CStrRef         GetSessionName() const;
    virtual void            SetSessionName(string sessionName);

    virtual CStrRef         GetInfo() const;
    virtual void            SetInfo(string info);

    virtual CStrRef         GetURI() const; 
    virtual void            SetURI(string uri);

    virtual ISDOrigin*      GetOrigin() const;
    virtual void            SetOrigin(ISDOrigin* origin);
    
    virtual ISDConnection*  GetConnection() const;
    virtual void            SetConnection(ISDConnection* conn);

    virtual ISDAttribute*   GetAttribute() const;
    //virtual CStrRef         GetAttribute(CStrRef name) const;
    //virtual void            SetAttribute(CStrRef name, CStrRef value);
    //virtual void            RemoveAttribute(CStrRef name);
    
    virtual StrContainer&   GetEmailList();
    //virtual void            SetEmailList(StrContainer & emailList);
    
    virtual StrContainer&   GetPhoneList();
    //virtual StrContainer&   SetPhoneList(StrContainer & phoneList);
    
    virtual TimeDescrContainer&     GetTimeDescriptionList();
    //virtual TimeDescrContainer&     SetTimeDescriptionList(TimeDescrContainer & timeDescList);
    
    virtual MediaDescrContainer&    GetMediaDescriptionList();
    //virtual MediaDescrContainer&    SetMediaDescriptionList(MediaDescrContainer & mediaDescList);

    virtual string          toString();

private:
    void                    CopySdpBase4Local2Final(SessionDescriptionImpl* finalSdp);
    ISDMediaDescription*    CopyMediaBase4Local2Final(ISDMediaDescription* lMediaDesc);
    bool                    FindMatchedMedia(const MediaDescrContainer& rMediaList, 
                                                       const MediaDescrContainer& lMediaList,
                                                       MediaDescrContainer& fMediaList);
    bool                    FindFirstMatchedCodec(ISDMediaDescription* rMediaDesc, 
                                                       ISDMediaDescription* lMediaDesc,
                                                       ISDMediaDescription** fMediaDesc,
                                                       MediaDescrContainer& fMediaList);
    //bool                    FindTelephoneEventCodec(ISDMediaDescription* rMediaDesc, 
    //                                                   ISDMediaDescription* lMediaDesc,
    //                                                   MediaDescrContainer& fMediaList);
    bool                    FindFirstMatchedCrypto(ISDMediaDescription* rMediaDesc, 
                                                       ISDMediaDescription* lMediaDesc,
                                                       ISDMediaDescription** fMediaDesc);
private:
    string              mContent;
    int                 mVersion;
    string              mSessionName;
    string              mInfo;
    string              mURI;
    ISDOrigin*          mOrigin;
    ISDConnection*      mConnection;
    SDAttributeImpl*    mAttribute;
    StrContainer        mEmailList;
    StrContainer        mPhoneList;
    TimeDescrContainer  mTimeDescriptionList;
    MediaDescrContainer mMediaDescriptionList;

};

#endif
