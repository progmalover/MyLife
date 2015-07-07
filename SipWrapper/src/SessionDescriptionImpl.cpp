#include "SessionDescriptionImpl.h"
#include "ConnectionImpl.h"
#include "MediaDescriptionImpl.h"
#include "ResipStackAdapterImpl.h"
#include "SipTrace.h"
#include <sstream>
using std::stringstream;
/////////////////////////////////////////////////////////////////////
////SDOriginImpl
//ISDOrigin* ISDOrigin::Create()
//{
//    return new SDOriginImpl();
//}

SDOriginImpl::SDOriginImpl() :
    mUserName("-"), mSessionID(0), mSessionVer(0),
    mNetType("IN"), mAddressType("IP4"), mAddress("")
{
    SIPSDK_CLASSTRACE("SDOriginImpl::SDOriginImpl");
}

SDOriginImpl::~SDOriginImpl()
{ 
    SIPSDK_CLASSTRACE("SDOriginImpl::~SDOriginImpl");
}

void SDOriginImpl::AddRef()
{
    CReferenceControl::add_reference();
}

void SDOriginImpl::ReleaseRef()
{
    CReferenceControl::release_reference();
}

CStrRef SDOriginImpl::GetUserName() const
{
    return this->mUserName;
}

void SDOriginImpl::SetUserName(string userName)
{
    this->mUserName = userName;
}

int64 SDOriginImpl::GetSessionID() const
{
    return this->mSessionID;
}

void SDOriginImpl::SetSessionID(int64 sessionId)
{
    this->mSessionID = sessionId;
}

int64 SDOriginImpl::GetSessionVer() const
{
    return this->mSessionVer;
}

void SDOriginImpl::SetSessionVer(int64 sessionVer)
{
    this->mSessionVer = sessionVer;
}

CStrRef SDOriginImpl::GetNetType() const
{
    return this->mNetType;
}

void SDOriginImpl::SetNetType(string netType)
{
    this->mNetType = netType;
}

CStrRef SDOriginImpl::GetAddressType() const
{
    return this->mAddressType;
}

void SDOriginImpl::SetAddressType(string addressType)
{
    this->mAddressType = addressType;
}

CStrRef SDOriginImpl::GetAddress() const
{
    return this->mAddress;
}

void SDOriginImpl::SetAddress(string address)
{
    this->mAddress = address;
}


/////////////////////////////////////////////////////////////////////
////SDRepeatTimeImpl
ISDRepeatTime* ISDRepeatTime::Create()
{
    return new SDRepeatTimeImpl();
}

SDRepeatTimeImpl::SDRepeatTimeImpl() : 
    mRepeatInterval(0), mActiveDuration(0), mOffset()
{
    SIPSDK_CLASSTRACE("SDRepeatTimeImpl::SDRepeatTimeImpl");
}

SDRepeatTimeImpl::~SDRepeatTimeImpl()
{ 
    SIPSDK_CLASSTRACE("SDRepeatTimeImpl::~SDRepeatTimeImpl");
}

int64 SDRepeatTimeImpl::GetRepeatInterval() const
{
    return this->mRepeatInterval;
}

void SDRepeatTimeImpl::SetRepeatInterval(int64 interval)
{
    this->mRepeatInterval = interval;
}

int64 SDRepeatTimeImpl::GetActiveDuration() const
{
    return this->mActiveDuration;
}

void SDRepeatTimeImpl::SetActiveDuration(int64 duration)
{
    this->mActiveDuration = duration;
}

vector<int> SDRepeatTimeImpl::GetOffset() const
{
    return this->mOffset;
}

void SDRepeatTimeImpl::SetOffset(const vector<int>& offset)
{
    this->mOffset.clear();
    this->mOffset.assign(offset.begin(), offset.end());
}

/////////////////////////////////////////////////////////////////////
////SDTimeDescriptionImpl
ISDTimeDescription* ISDTimeDescription::Create()
{
    return new SDTimeDescriptionImpl();
}

SDTimeDescriptionImpl::SDTimeDescriptionImpl() :
    mStartTime(0), mStopTime(0), mRepeatTime()
{
    SIPSDK_CLASSTRACE("SDTimeDescriptionImpl::SDTimeDescriptionImpl");
}

SDTimeDescriptionImpl::~SDTimeDescriptionImpl()
{ 
    SIPSDK_CLASSTRACE("SDTimeDescriptionImpl::~SDTimeDescriptionImpl");
    
    for_each(this->mRepeatTime.begin(), this->mRepeatTime.end(), SIPSDKDeleteObj());
    ISDTimeDescription::RTContainer().swap(this->mRepeatTime);
}

int64 SDTimeDescriptionImpl::GetStartTime() const
{
    return this->mStartTime;
}

void SDTimeDescriptionImpl::SetStartTime(int64 startTime)
{
    this->mStartTime = startTime;
}

int64 SDTimeDescriptionImpl::GetStopTime() const
{
    return this->mStopTime;
}

void SDTimeDescriptionImpl::SetStopTime(int64 stopTime)
{
    this->mStopTime = stopTime;
}

ISDTimeDescription::RTContainer& SDTimeDescriptionImpl::GetRepeatTimeList()
{
    return this->mRepeatTime;
}

/////////////////////////////////////////////////////////////////////
////SessionDescriptionImpl

SessionDescriptionImpl::SessionDescriptionImpl() :
    mContent(""),
    mVersion(0),    
    mSessionName("session"),
    mInfo(""),
    mURI(""),
    mOrigin(NULL),
    mConnection(NULL),
    mAttribute(NULL),
    mEmailList(),
    mPhoneList(),
    mTimeDescriptionList(),
    mMediaDescriptionList()
{
    SIPSDK_CLASSTRACE("SessionDescriptionImpl::SessionDescriptionImpl");

    this->mOrigin = new SDOriginImpl;
    this->mOrigin->AddRef();

    this->mConnection = new SDConnectionImpl;
    this->mConnection->AddRef();

    this->mAttribute = new SDAttributeImpl;
    this->mAttribute->AddRef();
    
    ISDTimeDescription* time = ISDTimeDescription::Create();
    mTimeDescriptionList.push_back(time);
}

SessionDescriptionImpl::~SessionDescriptionImpl()
{
    SIPSDK_CLASSTRACE("SessionDescriptionImpl::~SessionDescriptionImpl");

    if(this->mOrigin)
    {
        this->mOrigin->ReleaseRef();
        this->mOrigin = NULL;
    }

    if(this->mConnection)
    {
        this->mConnection->ReleaseRef();
        this->mConnection = NULL;
    }

    if(this->mAttribute)
    {
        this->mAttribute->ReleaseRef();
        this->mAttribute = NULL;
    }

    for_each(this->mTimeDescriptionList.begin(), this->mTimeDescriptionList.end(), SIPSDKDeleteObj());
    TimeDescrContainer().swap(this->mTimeDescriptionList);
    
    for_each(this->mMediaDescriptionList.begin(), this->mMediaDescriptionList.end(), SIPSDKDeleteObj());
    MediaDescrContainer().swap(this->mMediaDescriptionList);
}

/*
SessionDescriptionImpl& SessionDescriptionImpl::operator=(SessionDescriptionImpl& rhs)
{
    if(this == &rhs)
        return *this;
    

    this->mContent      = rhs.mContent;
    this->mVersion      = rhs.mVersion;
    this->mSessionName  = rhs.mSessionName;
    this->mInfo         = rhs.mInfo;
    this->mURI          = rhs.mURI;

    this->mOrigin->SetAddress(rhs.mOrigin->GetAddress());
    this->mOrigin->SetAddressType(rhs.mOrigin->GetAddressType());
    this->mOrigin->SetSessionID(rhs.mOrigin->GetSessionID());
    this->mOrigin->SetSessionVer(rhs.mOrigin->GetSessionVer());
    this->mOrigin->SetUserName(rhs.mOrigin->GetUserName());

    this->mConnection->SetAddress(rhs.mConnection->GetAddress());
    this->mConnection->SetAddressType(rhs.mConnection->GetAddressType());

    this->mAttribute->SetAttributeHelp(rhs.mAttribute->GetAttributeHelp());

    this->mEmailList = rhs.mEmailList;

    this->mPhoneList = rhs.mPhoneList;

    return *this;
}
*/

void SessionDescriptionImpl::AddRef()
{
    //DETAILTRACE("SessionDescriptionImpl::AddRef, this:" << this);
    CReferenceControl::add_reference();
}

void SessionDescriptionImpl::ReleaseRef()
{
    //DETAILTRACE("SessionDescriptionImpl::ReleaseRef, this:" << this);
    CReferenceControl::release_reference();
}

CStrRef SessionDescriptionImpl::GetRawContent() const
{
    //DETAILTRACE("SessionDescriptionImpl::GetRawContent, this:" << this);
    return this->mContent;
}

void SessionDescriptionImpl::SetRawContent(string content)
{
    //DETAILTRACE("SessionDescriptionImpl::SetRawContent, this:" << this);
    this->mContent = content;
}

int SessionDescriptionImpl::GetVersion() const
{
    //DETAILTRACE("SessionDescriptionImpl::GetVersion, this:" << this);
    return this->mVersion;
}

void SessionDescriptionImpl::SetVersion(int version)
{
    //DETAILTRACE("SessionDescriptionImpl::SetVersion, this:" << this);
    this->mVersion = version;
}

CStrRef SessionDescriptionImpl::GetSessionName() const
{
    //DETAILTRACE("SessionDescriptionImpl::GetSessionName, this:" << this);
    return this->mSessionName;
}

void SessionDescriptionImpl::SetSessionName(string sessionName)
{
    //DETAILTRACE("SessionDescriptionImpl::SetSessionName, this:" << this);
    this->mSessionName = sessionName;
}

CStrRef SessionDescriptionImpl::GetInfo() const
{
    //DETAILTRACE("SessionDescriptionImpl::GetInfo, this:" << this);
    return this->mInfo;
}

void SessionDescriptionImpl::SetInfo(string info)
{
    //DETAILTRACE("SessionDescriptionImpl::SetInfo, this:" << this);
    this->mInfo = info;
}

CStrRef SessionDescriptionImpl::GetURI() const
{
    //DETAILTRACE("SessionDescriptionImpl::GetURI, this:" << this);
    return this->mURI;
}

void SessionDescriptionImpl::SetURI(string uri)
{
    //DETAILTRACE("SessionDescriptionImpl::GetURI, this:" << this);
    this->mURI = uri;
}

ISDOrigin* SessionDescriptionImpl::GetOrigin() const
{
    //DETAILTRACE("SessionDescriptionImpl::GetOrigin, this:" << this);
    return this->mOrigin;
}

void SessionDescriptionImpl::SetOrigin(ISDOrigin* origin)
{
    //DETAILTRACE("SessionDescriptionImpl::SetOrigin, mOrigin:" << mOrigin << ", origin:" << origin);
    SIPSDK_ASSERT_RETURN_VOID(origin);
    
    if(this->mOrigin)
        this->mOrigin->ReleaseRef();

    this->mOrigin = origin;
    this->mOrigin->AddRef();
}

ISDConnection* SessionDescriptionImpl::GetConnection() const
{
    //DETAILTRACE("SessionDescriptionImpl::GetConnection, this:" << this);
    return this->mConnection;
}

void SessionDescriptionImpl::SetConnection(ISDConnection* conn)
{
    //DETAILTRACE("SessionDescriptionImpl::SetConnection, this:" << this);
    //return;
    
    SIPSDK_ASSERT_RETURN_VOID(conn);
    
    if(this->mConnection)
        this->mConnection->ReleaseRef();

    this->mConnection = conn;
    this->mConnection->AddRef();
}

ISDAttribute* SessionDescriptionImpl::GetAttribute() const
{
    //DETAILTRACE("SessionDescriptionImpl::GetAttribute, this:" << this);
    return this->mAttribute;
}
/*
void SessionDescriptionImpl::SetAttribute(CStrRef name, CStrRef value)
{
}

CStrRef SessionDescriptionImpl::GetAttribute(CStrRef name) const
{
    return this->mAttribute;
}

void SessionDescriptionImpl::RemoveAttribute(CStrRef name)
{
}
*/
StrContainer& SessionDescriptionImpl::GetEmailList() 
{
    //DETAILTRACE("SessionDescriptionImpl::GetEmailList, this:" << this);
    return this->mEmailList;
}

StrContainer& SessionDescriptionImpl::GetPhoneList() 
{
    //DETAILTRACE("SessionDescriptionImpl::GetPhoneList, this:" << this);
    return this->mPhoneList;
}

ISessionDescription::TimeDescrContainer& SessionDescriptionImpl::GetTimeDescriptionList() 
{
    //DETAILTRACE("SessionDescriptionImpl::GetTimeDescriptionList, this:" << this);
    return this->mTimeDescriptionList;
}

ISessionDescription::MediaDescrContainer& SessionDescriptionImpl::GetMediaDescriptionList() 
{
    //DETAILTRACE("SessionDescriptionImpl::GetMediaDescriptionList, this:" << this);
    return this->mMediaDescriptionList;
}

string SessionDescriptionImpl::toString()
{
    //DETAILTRACE("SessionDescriptionImpl::toString, this:" << this);
    stringstream sdp;
    sdp << "v=" << this->GetVersion() << Symbols::CRLF;

    if(this->GetOrigin())
    {
        sdp << "o=" << this->GetOrigin()->GetUserName() << " " 
            << this->GetOrigin()->GetSessionID() << " " << this->GetOrigin()->GetSessionVer()
            << " " << this->GetOrigin()->GetNetType() << " "<< this->GetOrigin()->GetAddressType()<< " " << this->GetOrigin()->GetAddress() << Symbols::CRLF;
    }

    sdp << "s=" << this->GetSessionName() << Symbols::CRLF;

    if(!this->GetInfo().empty())
        sdp << "i=" << this->GetInfo() << Symbols::CRLF;

    ISDConnection* conn = this->GetConnection();
    if(conn && !conn->GetAddress().empty())
        sdp << "c=" << conn->GetNetType() << " " << conn->GetAddressType() << " " << conn->GetAddress() << Symbols::CRLF;
    
    ISessionDescription::TimeDescrContainer& timeDescList = this->GetTimeDescriptionList();
    for (ISessionDescription::TimeDescrContainer::iterator iter = timeDescList.begin();
                                                           iter != timeDescList.end();
                                                           ++ iter) 
    {
        ISDTimeDescription* timeDesc = *iter;
        sdp << "t=" << timeDesc->GetStartTime() << " " << timeDesc->GetStopTime() << Symbols::CRLF;
    }

    // session attribute
    SDAttributeImpl* attPtr = static_cast<SDAttributeImpl*>(this->GetAttribute());
    if(attPtr != NULL)
    {
        AttributeHelper attHelp = attPtr->GetAttributeHelp();
        std::ostringstream ss;
        attHelp.encode(ss);
        sdp << ss.str();
    }

    ISessionDescription::MediaDescrContainer& mediaDescList = this->GetMediaDescriptionList();
    for (ISessionDescription::MediaDescrContainer::iterator iter = mediaDescList.begin();
                                                            iter != mediaDescList.end();
                                                            ++ iter) 
    {
        SDMediaDescriptionImpl* mediaDesc = static_cast<SDMediaDescriptionImpl*>(*iter);
        sdp << mediaDesc->toString();
    }

    return sdp.str();
}

ISessionDescription* SessionDescriptionImpl::Negotiate(ISessionDescription* remoteSdp) 
{
    SIPSDK_ASSERT_RETURN(remoteSdp, NULL);
    
    SIPSDK_DETAILTRACE("SessionDescriptionImpl::Negotiate, remoteSdp:" << remoteSdp << ", localSdp:" << this);
    SIPSDK_DETAILTRACE("SessionDescriptionImpl::Negotiate, remoteSdp:" << remoteSdp->toString());
    SIPSDK_DETAILTRACE("SessionDescriptionImpl::Negotiate, localSdp:" << this->toString());

    ISessionDescription::MediaDescrContainer& rMediaList = remoteSdp->GetMediaDescriptionList();
    ISessionDescription::MediaDescrContainer& lMediaList = this->GetMediaDescriptionList();

    // make sure local sdp MUST has "m=" row
    SIPSDK_ASSERT_RETURN(lMediaList.size() > 0, NULL);

    SessionDescriptionImpl* finalSdp = new SessionDescriptionImpl;

    // generate final sdp(except "m="), copy local to final
    this->CopySdpBase4Local2Final(finalSdp);

    // if remote sdp has no "m=" row, negotiate successfully. But the final sdp also keep no "m=" row
    if(rMediaList.size() == 0)
        return finalSdp;

    // find the first matched codec
    MediaDescrContainer& fMediaList = finalSdp->GetMediaDescriptionList();
    for_each(fMediaList.begin(), fMediaList.end(), SIPSDKDeleteObj());
    MediaDescrContainer().swap(fMediaList);
    
    bool bFound = this->FindMatchedMedia(rMediaList, lMediaList, fMediaList);

    if(!bFound)
        SIPSDK_SAFE_DELETE(finalSdp);

    SIPSDK_DETAILTRACE("SessionDescriptionImpl::Negotiate, finalSdp:" << finalSdp);
    if(finalSdp)
        SIPSDK_DETAILTRACE("SessionDescriptionImpl::Negotiate, finalSdp:" << finalSdp->toString());
    
    return finalSdp;
}

void SessionDescriptionImpl::CopySdpBase4Local2Final(SessionDescriptionImpl* finalSdp)
{
    //DETAILTRACE("SessionDescriptionImpl::CopySdpBase4Local2Final, this:" << this);
    finalSdp->mContent      = this->mContent;
    finalSdp->mVersion      = this->mVersion;
    finalSdp->mSessionName  = this->mSessionName;
    finalSdp->mInfo         = this->mInfo;
    finalSdp->mURI          = this->mURI;
    finalSdp->mOrigin->SetAddress(this->mOrigin->GetAddress());
    finalSdp->mOrigin->SetAddressType(this->mOrigin->GetAddressType());
    finalSdp->mOrigin->SetSessionID(this->mOrigin->GetSessionID());
    finalSdp->mOrigin->SetSessionVer(this->mOrigin->GetSessionVer());
    finalSdp->mOrigin->SetUserName(this->mOrigin->GetUserName());
    finalSdp->mConnection->SetAddress(this->mConnection->GetAddress());
    finalSdp->mConnection->SetAddressType(this->mConnection->GetAddressType());
    finalSdp->mAttribute->SetAttributeHelp(this->mAttribute->GetAttributeHelp());
    finalSdp->mEmailList = this->mEmailList;
    finalSdp->mPhoneList = this->mPhoneList;

    TimeDescrContainer& finalTimeDescList = finalSdp->GetTimeDescriptionList();
    for_each(finalTimeDescList.begin(), finalTimeDescList.end(), SIPSDKDeleteObj());
    ISessionDescription::TimeDescrContainer().swap(finalTimeDescList);
    
    TimeDescrContainer& localTimeDescList = this->GetTimeDescriptionList();
    TimeDescrContainerIter iter = localTimeDescList.begin();
    for(; iter != localTimeDescList.end(); ++ iter)
    {
        ISDTimeDescription* localTimeDesc = *iter; 
        ISDTimeDescription* finalTimeDesc = ISDTimeDescription::Create(); 
        //finalTimeDesc->AddRef();
        finalTimeDesc->SetStartTime(localTimeDesc->GetStartTime());
        finalTimeDesc->SetStopTime(localTimeDesc->GetStopTime());

        vector<ISDRepeatTime*>& localRTList = localTimeDesc->GetRepeatTimeList();
        vector<ISDRepeatTime*>& finalRTList = finalTimeDesc->GetRepeatTimeList();
        for_each(finalRTList.begin(), finalRTList.end(), SIPSDKDeleteObj());
        vector<ISDRepeatTime*>().swap(finalRTList);
    
        vector<ISDRepeatTime*>::iterator iter2 = localRTList.begin();
        for(; iter2 != localRTList.end(); ++ iter2)
        {
            ISDRepeatTime* localRT = *iter2;
            ISDRepeatTime* finalRT = ISDRepeatTime::Create();
            //finalRT->AddRef();

            finalRT->SetActiveDuration(localRT->GetActiveDuration());
            finalRT->SetRepeatInterval(localRT->GetRepeatInterval());
            finalRT->SetOffset(localRT->GetOffset());
            
            finalRTList.push_back(finalRT);
        }
        
        finalTimeDescList.push_back(finalTimeDesc);
    }
}

bool SessionDescriptionImpl::FindMatchedMedia(const MediaDescrContainer& rMediaList,
                                                        const MediaDescrContainer& lMediaList,
                                                        MediaDescrContainer& fMediaList)
{
    SIPSDK_DETAILTRACE("SessionDescriptionImpl::FindMatchedMedia, this:" << this);
    bool bFound = false; 
    
    string rtp_savp("RTP/SAVP");
    ISessionDescription::MediaDescrContainer::const_iterator rIter = rMediaList.begin();
    ISessionDescription::MediaDescrContainer::const_iterator lIter = lMediaList.begin();

    // work through both media list, find the first matched codec for every media
    for(; rIter != rMediaList.end(); ++ rIter)
    {
        ISDMediaDescription* rMediaDesc = *rIter;

        lIter = lMediaList.begin();
        for(; lIter != lMediaList.end(); ++ lIter)
        {
            ISDMediaDescription* lMediaDesc = *lIter;

            // Modified by Sam, 08/31/2011 for SRTP audio
            // loop through media list, require media type and media protocol have the same value
            if(!is_equals(rMediaDesc->GetMediaType(), lMediaDesc->GetMediaType()) ||
               !is_equals(rMediaDesc->GetProtocol(),  lMediaDesc->GetProtocol()) )
                continue;

            ISDMediaDescription* fMediaDesc = NULL;
            if(is_equals(rMediaDesc->GetProtocol(), rtp_savp))
            {
                bool foundCrypto = this->FindFirstMatchedCrypto(rMediaDesc, lMediaDesc, &fMediaDesc);
                if(!foundCrypto)
                    continue;
            }

            // find the first codec
            bool bMatchFirstCodec = false;
            bMatchFirstCodec = this->FindFirstMatchedCodec(rMediaDesc, lMediaDesc, &fMediaDesc, fMediaList);

            bFound = bMatchFirstCodec ? true : bFound;

            if(bMatchFirstCodec)
                break;
        }
    }

    return bFound;
}

bool SessionDescriptionImpl::FindFirstMatchedCrypto(ISDMediaDescription* rMediaDesc,
                                                        ISDMediaDescription* lMediaDesc,
                                                        ISDMediaDescription** fMediaDesc)
{
    SIPSDK_DETAILTRACE("SessionDescriptionImpl::FindFirstMatchedCrypto, this:" << this);
    SIPSDK_ASSERT_RETURN(rMediaDesc, false);
    SIPSDK_ASSERT_RETURN(lMediaDesc, false);

    bool bFound = false;
    
    vector<ISDCrypto*>& rCryptoList = rMediaDesc->GetAllCryptos();
    vector<ISDCrypto*>& lCryptoList = lMediaDesc->GetAllCryptos();
            
    vector<ISDCrypto*>::iterator rCryptoIter = rCryptoList.begin();
    for(; rCryptoIter != rCryptoList.end(); ++ rCryptoIter)
    {
        ISDCrypto* rCrypto = *rCryptoIter;
        
        vector<ISDCrypto*>::iterator lCryptoIter = lCryptoList.begin();
        for(; lCryptoIter != lCryptoList.end(); ++ lCryptoIter)
        {
            ISDCrypto* lCrypto = *lCryptoIter;

            if(!is_equals(rCrypto->GetCryptoSuiteType(), lCrypto->GetCryptoSuiteType()))
                continue;

            ISDMediaDescription* finalDesc = this->CopyMediaBase4Local2Final(lMediaDesc);
            *fMediaDesc = finalDesc;
                
            // crypto
            vector<ISDCrypto*>& finalCryptoList = finalDesc->GetAllCryptos();
            SDCryptoImpl* fCrypto = static_cast<SDCryptoImpl*>(ISDCrypto::Create());
            finalCryptoList.push_back(fCrypto);

            fCrypto->SetCryptoSuiteType(rCrypto->GetCryptoSuiteType());
            fCrypto->SetTag(rCrypto->GetTag());

            vector<ISDCryptoParam*>& fParams = fCrypto->GetAllParams();
            vector<ISDCryptoParam*>& lParams = lCrypto->GetAllParams();

            for(vector<ISDCryptoParam*>::iterator iter = lParams.begin();
                                                  iter != lParams.end();
                                                  ++ iter)
            {
                SDCryptoParamImpl* Param = static_cast<SDCryptoParamImpl*>(*iter);
                SDCryptoParamImpl* newParam = Param->Clone();
                fParams.push_back(newParam);
            }
            SDMediaDescriptionImpl *tmp = (SDMediaDescriptionImpl*)finalDesc;
            cout << "final media 1111: " << tmp->toString() << endl;
            bFound = true;
            break;
        }

        if(bFound)
            break;
    }

    return bFound;
}

// Update by Sam, 08/08/2011. "telephone-event" also added into return list
bool SessionDescriptionImpl::FindFirstMatchedCodec(ISDMediaDescription* rMediaDesc,
                                                        ISDMediaDescription* lMediaDesc,
                                                        ISDMediaDescription** fMediaDesc,
                                                        MediaDescrContainer& fMediaList)
{
    SIPSDK_DETAILTRACE("SessionDescriptionImpl::FindFirstMatchedCodec, this:" << this);
    SIPSDK_ASSERT_RETURN(rMediaDesc, false);
    SIPSDK_ASSERT_RETURN(lMediaDesc, false);
    
    bool bFoundTelephoneEvtCodec = false;
    bool bFoundFirstOtherCodec = false;
    
    string t_evt("telephone-event");

    ISDMediaDescription* finalMediaDesc = *fMediaDesc;
    // generate the negotiable media part into final sdp
    //MediaDescrContainer& finalMediaDescList = finalSdp->GetMediaDescriptionList();
    if(finalMediaDesc == NULL)
        finalMediaDesc = this->CopyMediaBase4Local2Final(lMediaDesc);
    fMediaList.push_back(finalMediaDesc);
    
    vector<ISDCodec*>& rCodecList = rMediaDesc->GetAllCodecs();
    vector<ISDCodec*>& lCodecList = lMediaDesc->GetAllCodecs();
            
    vector<ISDCodec*>::iterator rCodecIter = rCodecList.begin();
    for(; rCodecIter != rCodecList.end(); ++ rCodecIter)
    {
        ISDCodec* rCodec = *rCodecIter;
        
        vector<ISDCodec*>::iterator lCodecIter = lCodecList.begin();
        for(; lCodecIter != lCodecList.end(); ++ lCodecIter)
        {
            ISDCodec* lCodec = *lCodecIter;

            if(!is_equals(lCodec->GetName(), rCodec->GetName()))
                continue;
        
            if(bFoundTelephoneEvtCodec && is_equals(rCodec->GetName(), t_evt))
                continue;

            if(bFoundFirstOtherCodec && !is_equals(rCodec->GetName(), t_evt))
                continue;
            
            if(is_equals(rCodec->GetName(), t_evt))
                bFoundTelephoneEvtCodec = true;
            else
                bFoundFirstOtherCodec = true;

            // media payload type list
            vector<int>& finalPayLoadTypeList = finalMediaDesc->GetPayLoadTypeList();
            finalPayLoadTypeList.push_back(lCodec->GetPayloadType());

            // the first matched codec
            vector<ISDCodec*>& finalCodecList = finalMediaDesc->GetAllCodecs();
            SDCodecImpl* finalCodec = static_cast<SDCodecImpl*>(ISDCodec::Create());
            //finalCodec->AddRef();
            finalCodecList.push_back(finalCodec);
            *finalCodec = *(static_cast<SDCodecImpl*>(lCodec));
            // end generate
        }

        // just return the first matched codec for every media
        if (bFoundTelephoneEvtCodec && bFoundFirstOtherCodec)
            break;
    }

    return bFoundTelephoneEvtCodec || bFoundFirstOtherCodec;
}

ISDMediaDescription* SessionDescriptionImpl::CopyMediaBase4Local2Final(ISDMediaDescription* lMediaDesc)
{
    ISDMediaDescription* finalMediaDesc = ISDMediaDescription::Create();
    
    // media base info
    finalMediaDesc->SetInfo(lMediaDesc->GetInfo());
    finalMediaDesc->SetMediaPort(lMediaDesc->GetMediaPort());
    finalMediaDesc->SetMediaType(lMediaDesc->GetMediaType());
    finalMediaDesc->SetProtocol(lMediaDesc->GetProtocol());

    // media connection
    vector<ISDConnection*>& localConnList = lMediaDesc->GetConnectionList();
    vector<ISDConnection*>& finalConnList = finalMediaDesc->GetConnectionList();
    if(localConnList.size() > 0)
    {
        vector<ISDConnection*>::iterator connIter = localConnList.begin();
        SDConnectionImpl* localConn = static_cast<SDConnectionImpl*>(*connIter);
        SDConnectionImpl* finalConn = static_cast<SDConnectionImpl*>(ISDConnection::Create());
        //finalConn->AddRef();
        finalConnList.push_back(finalConn);
        *finalConn = *localConn;
    }

    return finalMediaDesc;
}

ISessionDescription* SessionDescriptionImpl::Clone() 
{
    SessionDescriptionImpl* newSdp = new SessionDescriptionImpl;
    
    newSdp->mContent        = this->mContent;
    newSdp->mVersion        = this->mVersion;
    newSdp->mSessionName    = this->mSessionName;
    newSdp->mInfo           = this->mInfo;
    newSdp->mURI            = this->mURI;

    newSdp->mOrigin->SetAddress(this->mOrigin->GetAddress());
    newSdp->mOrigin->SetAddressType(this->mOrigin->GetAddressType());
    newSdp->mOrigin->SetNetType(this->mOrigin->GetNetType());
    newSdp->mOrigin->SetSessionID(this->mOrigin->GetSessionID());
    newSdp->mOrigin->SetSessionVer(this->mOrigin->GetSessionVer());
    newSdp->mOrigin->SetUserName(this->mOrigin->GetUserName());

    newSdp->mConnection->SetAddress(this->mConnection->GetAddress());
    newSdp->mConnection->SetAddressType(this->mConnection->GetAddressType());
    newSdp->mConnection->SetNetType(this->mConnection->GetNetType());
        
    newSdp->mAttribute->SetAttributeHelp(this->mAttribute->GetAttributeHelp());
    newSdp->mEmailList.assign(this->mEmailList.begin(), this->mEmailList.end());
    newSdp->mPhoneList.assign(this->mPhoneList.begin(), this->mPhoneList.end());

    for_each(newSdp->mTimeDescriptionList.begin(), newSdp->mTimeDescriptionList.end(), SIPSDKDeleteObj());
    TimeDescrContainer().swap(newSdp->mTimeDescriptionList);
    
    for_each(newSdp->mMediaDescriptionList.begin(), newSdp->mMediaDescriptionList.end(), SIPSDKDeleteObj());
    MediaDescrContainer().swap(newSdp->mMediaDescriptionList);
                             
    for(TimeDescrContainer::iterator iter = this->mTimeDescriptionList.begin();
                                     iter != this->mTimeDescriptionList.end();
                                     ++iter)
    {
        ISDTimeDescription* time = *iter;
        ISDTimeDescription* newTime = ISDTimeDescription::Create();

        newTime->SetStartTime(time->GetStartTime());
        newTime->SetStopTime(time->GetStopTime());
        //TBD... repeat time

        newSdp->mTimeDescriptionList.push_back(newTime);
    }
    
    for(MediaDescrContainer::iterator iter = this->mMediaDescriptionList.begin();
                                      iter != this->mMediaDescriptionList.end();
                                      ++iter)
    {
        SDMediaDescriptionImpl* mediaDesc = static_cast<SDMediaDescriptionImpl*>(*iter);
        SDMediaDescriptionImpl* newMediaDesc = mediaDesc->Clone();
        newSdp->mMediaDescriptionList.push_back(newMediaDesc);
    }
    
    return newSdp;
}

ISessionDescription* ISessionDescription::Create(string * sdp)
{
    //DETAILTRACE("SessionDescriptionImpl::Create");
    if(sdp)
        return SessionDescriptionImpl::Create(*sdp);

    else
        return new SessionDescriptionImpl;
}

SessionDescriptionImpl * SessionDescriptionImpl::Create(CStrRef rawContent)
{
    HeaderFieldValue hfv(rawContent.c_str(), rawContent.size());
    Mime type("application", "sdp");
    SdpContents* sdp = new SdpContents(&hfv, type);
    auto_ptr<SdpContents> autoDel(sdp);

    try 
    {
        SIPSDK_AUTO_REF<SessionDescriptionImpl> sdkSDP(new SessionDescriptionImpl());
        
        // fill sdp raw content
        sdkSDP->SetRawContent(rawContent.c_str());

        ResipSipMessage obj;
        // fill sdp base info
        CmResult ret = obj.FillSdpBase2SDK(sdp, sdkSDP.get());
        SIPSDK_ASSERT_RETURN(SIPSDK_ERROR_NOERROR == ret, NULL);

        // fill sdp media info
        ISessionDescription::MediaDescrContainer& sdkMediaDescList = sdkSDP->GetMediaDescriptionList();
        ret = obj.FillSdpMedia2SDK(sdp,  sdkMediaDescList);
        SIPSDK_ASSERT_RETURN(SIPSDK_ERROR_NOERROR == ret, NULL);

        return sdkSDP.release();
    } 
    catch (exception& e)
    {
        SIPSDK_ERRTRACE("ISessionDescription::Create, Parse sdp failed, e.what(): " << e.what());
    }

    return NULL;
}

