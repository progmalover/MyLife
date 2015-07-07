#include "SipMessageImpl.h"
#include "SessionDescriptionImpl.h"
#include "SipTrace.h"
#include <stdio.h>
#include <sstream>
using std::stringstream;


ISipUri * ISipUri::newInstance()
{
    return new SipUriImpl(NULL);
}

ISipContact * ISipContact::newInstance()
{
    return new SipContactImpl();
}

void DestroyContactList(ContactList & list)
{
    ContactList::iterator it = list.begin();
    for(; it != list.end(); ++ it)
        (*it)->ReleaseRef();

    list.clear();
}

/////////////////////////////////////////////////////////////////////
////SipFromImpl
SipUriImpl::SipUriImpl(SipUriUpdateListener* listener) : 
    mUri(),
    mUpdateListener(listener)
{
    SIPSDK_CLASSTRACE("SipUriImpl::SipUriImpl");
}

SipUriImpl::SipUriImpl(SipUriUpdateListener* listener, const Uri& uri) : 
    mUri(uri),
    mUpdateListener(listener)
{
    SIPSDK_CLASSTRACE("SipUriImpl::SipUriImpl");
}

SipUriImpl::~SipUriImpl()
{ 
    SIPSDK_CLASSTRACE("SipUriImpl::~SipUriImpl");
}

void SipUriImpl::AddRef()
{
    CReferenceControl::add_reference();
}

void SipUriImpl::ReleaseRef()
{
    CReferenceControl::release_reference();
}

string SipUriImpl::GetScheme()
{
    return mUri.scheme().c_str();
}

void SipUriImpl::SetScheme(string scheme)
{
    mUri.scheme() = scheme.c_str();
    this->NotifyListener();
}
    
string SipUriImpl::GetUser()
{
    return mUri.user().c_str();
}

void SipUriImpl::SetUser(string user)
{
    mUri.user() = user.c_str();
    this->NotifyListener();
}

string SipUriImpl::GetPassword()
{
    return mUri.password().c_str();
}

void SipUriImpl::SetPassword(string pass)
{
    mUri.password() = pass.c_str();
    this->NotifyListener();
}

string SipUriImpl::GetHost()
{
    return mUri.host().c_str();
}

void SipUriImpl::SetHost(string host)
{
    mUri.host() = host.c_str();
    this->NotifyListener();
}

int SipUriImpl::GetPort()
{
    return mUri.port();
}

void SipUriImpl::SetPort(int port)
{
    mUri.port() = port;
    this->NotifyListener();
}

bool SipUriImpl::HasParam(string name)
{
    Data name_data(name);
    
    ParameterTypes::Type param_type = ParameterTypes::getType(name_data.data(), (unsigned int)name_data.size());
    if(param_type != ParameterTypes::UNKNOWN)
    {
        Parameter* parameter = mUri.getParameterByEnum(param_type);
        if(parameter)
            return true;
    }
    else
    {
        ExtensionParameter ext_param(name_data);
        return mUri.exists(ext_param);
    }
    
    return false;
}

string SipUriImpl::GetDataParam(string name)
{
    Data name_data(name);
    
    ParameterTypes::Type param_type = ParameterTypes::getType(name_data.data(), (unsigned int)name_data.size());
    if(param_type != ParameterTypes::UNKNOWN)
    {
        //"user", "transport", "maddr", "comp", "method"
        if(strcasecmp(name.c_str(), "user") == 0)
            return  mUri.param(p_user).c_str();
            
        else if(strcasecmp(name.c_str(), "transport") == 0)
            return  mUri.param(p_transport).c_str();
            
        else if(strcasecmp(name.c_str(), "maddr") == 0)
            return  mUri.param(p_maddr).c_str();
            
        else if(strcasecmp(name.c_str(), "comp") == 0)
            return  mUri.param(p_comp).c_str();
            
        else if(strcasecmp(name.c_str(), "method") == 0)
            return  mUri.param(p_method).c_str();
            
        else if(strcasecmp(name.c_str(), "ttl") == 0)
        {
            stringstream os;
            os << mUri.param(p_ttl);
            
            return os.str();
        }
        
        else if(strcasecmp(name.c_str(), "lr") == 0)
        {
            stringstream os;
            if(mUri.exists(p_lr)) os << "true";
            else                  os << "false";
            
            return os.str();
        }
    }
    
    return "";
}

void SipUriImpl::SetDataParam(string name, string value)
{
    Data name_data(name);
    
    ParameterTypes::Type param_type = ParameterTypes::getType(name_data.data(), (unsigned int)name_data.size());
    if(param_type != ParameterTypes::UNKNOWN)
    {
        //"user", "transport", "maddr", "comp", "method"
        if(strcasecmp(name.c_str(), "user") == 0)
            mUri.param(p_user) = Data(value);
            
        else if(strcasecmp(name.c_str(), "transport") == 0)
            mUri.param(p_transport) = Data(value);
            
        else if(strcasecmp(name.c_str(), "maddr") == 0)
            mUri.param(p_maddr) = Data(value);
            
        else if(strcasecmp(name.c_str(), "comp") == 0)
            mUri.param(p_comp) = Data(value);
            
        else if(strcasecmp(name.c_str(), "method") == 0)
            mUri.param(p_method) = Data(value);
        
        else if(strcasecmp(name.c_str(), "ttl") == 0)
            mUri.param(p_ttl) = atoi(value.c_str());
            
        else if(strcasecmp(name.c_str(), "lr") == 0)
        {
            if(strcasecmp(value.c_str(), "true") == 0)
                mUri.param(p_lr) = true;
            else
                mUri.remove(p_lr);
        }
    }
}

string SipUriImpl::GetUnknownParam(string name)
{
    Data name_data(name);
    ParameterTypes::Type param_type = ParameterTypes::getType(name_data.data(), (unsigned int)name_data.size());
    if(param_type == ParameterTypes::UNKNOWN)
        return mUri.param(UnknownParameterType(name_data)).c_str();
    
    return "";
}

void SipUriImpl::SetUnknownParam(string name, string value)
{
    Data name_data(name);
    
    ParameterTypes::Type param_type = ParameterTypes::getType(name_data.data(), (unsigned int)name_data.size());
    if(param_type == ParameterTypes::UNKNOWN)
    {
        mUri.param(UnknownParameterType(name_data)) = Data(value);
        return;
    }
}

void SipUriImpl::SetUri(const Uri & uri)
{
    mUri = uri;
}

const Uri & SipUriImpl::GetUri() const
{
    return mUri;
}

string SipUriImpl::toString()
{
    return mUri.toString().c_str();
}

bool SipUriImpl::fromString(CStrRef str)
{
    Data data(str.c_str());

    try
    {
        ParseBuffer pb(data);
        
        Uri tmp;
        tmp.parse(pb);
        
        mUri = tmp;
    }
    catch (ParseException & e) 
    {
        SIPSDK_ERRTRACE("SipUriImpl::fromString, parse failed! str: " << str);
        return false;
    }
    
    return true;
}

void SipUriImpl::NotifyListener()
{
    if(mUpdateListener)
        mUpdateListener->UpdateUriHandler();
}

/////////////////////////////////////////////////////////////////////
////SipFromImpl
SipFromImpl::SipFromImpl() :
    mTag(""), mFrom(""), mContent(""), mFromUri(NULL)
{
    SIPSDK_CLASSTRACE("SipFromImpl::SipFromImpl");

    mFromUri = new SipUriImpl(this);
    mFromUri->AddRef();
}

SipFromImpl::SipFromImpl(CStrRef content) :
    mTag(""), mFrom(""), mContent(content), mFromUri(NULL)
{
    SIPSDK_CLASSTRACE("SipFromImpl::SipFromImpl");

    mFromUri = new SipUriImpl(this);
    mFromUri->AddRef();
    
    this->Decode();
    this->ConvertStr2Uri();
}
    
SipFromImpl::~SipFromImpl()
{ 
    SIPSDK_CLASSTRACE("SipFromImpl::~SipFromImpl");

    if(mFromUri)
    {
        mFromUri->ReleaseRef();
        mFromUri = NULL;
    }
}

void SipFromImpl::AddRef()
{
    CReferenceControl::add_reference();
}

void SipFromImpl::ReleaseRef()
{
    CReferenceControl::release_reference();
}

// copy URI & tag
void SipFromImpl::Copy(ISipFrom * r)
{
    if(!r)
        return;

    SipFromImpl * rImpl = static_cast<SipFromImpl *>(r);
    if(!rImpl)
        return;

    mContent = rImpl->mContent;
    mTag = rImpl->mTag;
    mFrom = rImpl->mFrom;
    
    this->ConvertStr2Uri();
}

void SipFromImpl::UpdateUriHandler()
{
    this->ConvertUri2Str();
}

CStrRef SipFromImpl::GetTag() const
{
    return this->mTag;
}

void SipFromImpl::SetTag(string tag)
{
    this->mTag.assign(tag);
    this->Encode();
}

CStrRef SipFromImpl::GetFrom() const
{
    return this->mFrom;
}

void SipFromImpl::SetFrom(string from)
{
    this->mFrom.assign(from);
    this->Encode();
    this->ConvertStr2Uri();
}

ISipUri* SipFromImpl::GetFromUri() const
{
    return this->mFromUri;
}

void SipFromImpl::Decode()
{
    SIPSDK_ASSERT_RETURN_VOID(!this->mContent.empty())

    size_t pos = this->mContent.find(";");

    if(pos != string::npos)
    {
        string strFrom = this->mContent.substr(0, pos);
        string strTag = this->mContent.substr(pos + 1);

        this->mFrom.assign(TrimString(strFrom));
        this->mTag.clear();

        size_t pos1 = strTag.find("tag=");
        if(pos1 != string::npos)
            this->mTag.assign(strTag.substr(pos1+4));
    }
    else
    {
        this->mFrom.assign(this->mContent);
        this->mTag.clear();
    }
}

void SipFromImpl::Encode()
{
    SIPSDK_ASSERT_RETURN_VOID(!this->mFrom.empty());

    this->mContent.clear();
    this->mContent.assign(this->mFrom);
    
    if(!mTag.empty())
    {
        this->mContent.append(";tag=");
        this->mContent.append(this->mTag);
    }
}

void SipFromImpl::ConvertStr2Uri()
{
    this->mFromUri->fromString(this->mFrom);
}

void SipFromImpl::ConvertUri2Str()
{
    this->mFrom = this->mFromUri->toString().c_str();
    this->Encode();
}

CStrRef SipFromImpl::toString() const
{
    return this->mContent;
}

/////////////////////////////////////////////////////////////////////
////SipToImpl
SipToImpl::SipToImpl() :
    mTag(""), mTo(""), mContent(""), mToUri(NULL)
{
    SIPSDK_CLASSTRACE("SipToImpl::SipToImpl");

    mToUri = new SipUriImpl(this);
    mToUri->AddRef();
}

SipToImpl::SipToImpl(CStrRef content) :
    mTag(""), mTo(""), mContent(content), mToUri(NULL)
{
    SIPSDK_CLASSTRACE("SipToImpl::SipToImpl");

    mToUri = new SipUriImpl(this);
    mToUri->AddRef();
    
    this->Decode();
    this->ConvertStr2Uri();
}

SipToImpl::~SipToImpl()
{
    SIPSDK_CLASSTRACE("SipToImpl::~SipToImpl");

    if(mToUri)
    {
        mToUri->ReleaseRef();
        mToUri = NULL;
    }
}

void SipToImpl::AddRef()
{
    CReferenceControl::add_reference();
}

void SipToImpl::ReleaseRef()
{
    CReferenceControl::release_reference();
}

// copy URI & tag
void SipToImpl::Copy(ISipTo * r)
{
    if(!r)
        return;

    SipToImpl * rImpl = static_cast<SipToImpl *>(r);
    if(!rImpl)
        return;

    mContent = rImpl->mContent;
    mTag     = rImpl->mTag;
    mTo      = rImpl->mTo;
    
    this->ConvertStr2Uri();
}

void SipToImpl::UpdateUriHandler()
{
    this->ConvertUri2Str();
}

CStrRef SipToImpl::GetTag() const
{
    return this->mTag;
}

void SipToImpl::SetTag(string tag)
{
    this->mTag.assign(tag);
    this->Encode();
}

CStrRef SipToImpl::GetTo() const
{
    return this->mTo;
}

void SipToImpl::SetTo(string to)
{
    this->mTo.assign(to);
    this->Encode();
    this->ConvertStr2Uri();
}

ISipUri* SipToImpl::GetToUri() const
{
    return this->mToUri;
}

void SipToImpl::Decode()
{
    SIPSDK_ASSERT_RETURN_VOID(!this->mContent.empty())

    size_t pos = this->mContent.find(";");

    if(pos != string::npos)
    {
        string strTo = this->mContent.substr(0, pos);
        string strTag = this->mContent.substr(pos + 1);

        this->mTo.assign(TrimString(strTo));
        this->mTag.clear();

        size_t pos1 = strTag.find("tag=");
        if(pos1 != string::npos)
            this->mTag.assign(strTag.substr(pos1+4));
    }
    else
    {
        this->mTo.assign(this->mContent);
        this->mTag.clear();
    }
}

void SipToImpl::Encode()
{
    SIPSDK_ASSERT_RETURN_VOID(!this->mTo.empty());

    this->mContent.clear();
    this->mContent.assign(this->mTo);
    
    if(!mTag.empty())
    {
        this->mContent.append(";tag=");
        this->mContent.append(this->mTag);
    }
}

void SipToImpl::ConvertStr2Uri()
{
    this->mToUri->fromString(this->mTo);
}

void SipToImpl::ConvertUri2Str()
{
    this->mTo = this->mToUri->toString().c_str();
    this->Encode();
}

CStrRef SipToImpl::toString() const
{
    return this->mContent;
}

/////////////////////////////////////////////////////////////////////
////SipContactImpl
SipContactImpl::SipContactImpl()
               : mDisplayName(),
                 mHasParamQ(false),
                 mHasParamExpires(false),
                 mParamQ(1.0),
                 mParamExpires(0),
                 mContactUri(NULL)
{
    SIPSDK_CLASSTRACE("SipContactImpl::SipContactImpl");

    mContactUri = new SipUriImpl(this);
    mContactUri->AddRef();
}

SipContactImpl::~SipContactImpl()
{
    SIPSDK_CLASSTRACE("SipContactImpl::~SipContactImpl");

    if(mContactUri)
    {
        mContactUri->ReleaseRef();
        mContactUri = NULL;
    }
}

void SipContactImpl::AddRef()
{
    CReferenceControl::add_reference();
}

void SipContactImpl::ReleaseRef()
{
    CReferenceControl::release_reference();
}

CStrRef SipContactImpl::GetDisplayName() const
{
    return mDisplayName;
}

void SipContactImpl::SetDisplayName(CStrRef displayName)
{
    mDisplayName.assign(displayName);
}

// inherit from SipUriUpdateListener
void SipContactImpl::UpdateUriHandler()
{
}

string SipContactImpl::GetContact()
{
    return this->mContactUri->toString();
}

void SipContactImpl::SetContact(string uri)
{
   this->mContactUri->fromString(uri);
}

ISipUri* SipContactImpl::GetContactUri() const
{
    return this->mContactUri;
}

// Contact parameters "q" and "expires".
bool SipContactImpl::HasQValue() const
{
    return this->mHasParamQ;
}

float SipContactImpl::GetQValue() const
{
    return this->mParamQ;
}

void SipContactImpl::SetQValue(float p_q)
{
    this->mHasParamQ = true;
    this->mParamQ    = p_q;
}

bool SipContactImpl::HasExpires() const
{
    return this->mHasParamExpires;
}

int SipContactImpl::GetExpires() const
{
    return this->mParamExpires;
}

void SipContactImpl::SetExpires(int p_expires)
{
    this->mHasParamExpires = true;
    this->mParamExpires    = p_expires;
}

void SipContactImpl::toNameAddr(NameAddr & na)
{
    na.displayName() = Data(this->GetDisplayName());
    
    if(this->HasQValue())
        na.param(p_q) = this->GetQValue();
    
    if(this->HasExpires())
        na.param(p_expires) = this->GetExpires();
        
    na.uri() = mContactUri->GetUri();
}

// return sip from representation string, "<sip:alice@atlanta.com>;expires=3600"
string SipContactImpl::toString() 
{
    ostringstream str_buf;
    
    try
    {
        NameAddr na;
        this->toNameAddr(na);

        na.encodeParsed(str_buf);
    } catch(ParseException & e) {
        SIPSDK_ERRTRACE("SipContactImpl::toString(), this: " << this << ", error: " << e.what());
    }
 
    return str_buf.str();
}

bool SipContactImpl::fromString(CStrRef buf)
{
    // clear tags values   
    mParamQ          = 1.0;
    mHasParamQ       = false;
    mHasParamExpires = false;
    mParamExpires    = 0;
    
    Data data(buf);
    
    try
    {
        NameAddr na(data);
        
        if(na.exists(p_q))
        {
            mHasParamQ = true;
            mParamQ    = na.param(p_q);
        }
        
        if(na.exists(p_expires))
        {
            mHasParamExpires = true;
            mParamExpires    = na.param(p_expires);
        }
        
        mDisplayName.assign(na.displayName().c_str());
        mContactUri->SetUri(na.uri());
        
    } catch(ParseException & e) {
        SIPSDK_ERRTRACE("SipContactImpl::fromString(), buf: " << buf << ", what(): " << e.what());
        return false;
    }
    
    return true;
}

void SipContactImpl::AddNewContact (
                       ContactList & list,
                       CStrRef uri,
                       LPCSTR  display_name,
                       bool    has_expires,
                       int     p_expires,
                       bool    has_q,
                       float   p_q)
{
    ISipContact * contact = new SipContactImpl();
    contact->AddRef();
    
    if(!contact->GetContactUri()->fromString(uri))
    {
        SIPSDK_ERRTRACE("SipContactImpl::AddNewContact failed, uri: " << uri);
        contact->ReleaseRef();
        return;
    }
    
    SIPSDK_INFOTRACE("SipContactImpl::AddNewContact, uri: " << uri << ", display_name: " << display_name ? display_name : "");
    
    if(display_name)
    {
        string name(display_name);
        contact->SetDisplayName(name);
    }
    
    if(has_expires)
        contact->SetExpires(p_expires);
    
    if(has_q)
        contact->SetQValue(p_q);
    
    list.push_back(contact);
}

/////////////////////////////////////////////////////////////////////
////SipCSeqImpl
SipCSeqImpl::SipCSeqImpl() :
    mCSeq(""), mMethod(SIP_METHOD_UNKNOWN), mNum(-1)
{
    SIPSDK_CLASSTRACE("SipCSeqImpl::SipCSeqImpl");
}

SipCSeqImpl::SipCSeqImpl(CStrRef cseq) :
    mCSeq(cseq), mMethod(SIP_METHOD_UNKNOWN), mNum(-1)
{
    SIPSDK_CLASSTRACE("SipCSeqImpl::SipCSeqImpl");
    this->Decode();
}

SipCSeqImpl::~SipCSeqImpl()
{
    SIPSDK_CLASSTRACE("SipCSeqImpl::~SipCSeqImpl");
}

void SipCSeqImpl::AddRef()
{
    CReferenceControl::add_reference();
}

void SipCSeqImpl::ReleaseRef()
{
    CReferenceControl::release_reference();
}

void SipCSeqImpl::Decode()
{
    SIPSDK_ASSERT_RETURN_VOID(!this->mCSeq.empty())

    size_t pos = this->mCSeq.find(" ");
    SIPSDK_ASSERT_RETURN_VOID(pos != string::npos)

    string strNum = this->mCSeq.substr(0, pos);
    string strMethod = this->mCSeq.substr(pos + 1);

    this->mNum = ::atoi(strNum.c_str());
    this->mMethod = ::ConvertStr2Method(strMethod);
}

void SipCSeqImpl::Encode()
{
    SIPSDK_ASSERT_RETURN_VOID(mMethod != SIP_METHOD_UNKNOWN || mNum != -1);

    char szNum[16] = {0};
    ::sprintf(szNum, "%d", mNum);

    mCSeq.clear();
    mCSeq.append(szNum);
    mCSeq.append(" ");
    mCSeq.append(::ConvertMethod2Str(mMethod));
}

SipMethod SipCSeqImpl::GetMethod() const
{
    return this->mMethod;
}

void SipCSeqImpl::SetMethod(SipMethod method)
{
    this->mMethod = method;
    this->Encode();
}

int SipCSeqImpl::GetNumber() const
{
    return this->mNum;
}

void SipCSeqImpl::SetNumber(int num)
{
    this->mNum = num;
    this->Encode();
}

CStrRef SipCSeqImpl::toString() const
{
    return this->mCSeq;
}

/////////////////////////////////////////////////////////////////////
////SDKSipMessage
SDKSipMessage::SDKSipMessage() :
    mAlso(),
    mRefer(),
    mCallID(""),
    mContactList(),
    mCSeq(NULL),
    mFrom(NULL),
    mTo(NULL),
    mViaList(),
    mAllowEventList(),
    mRequireList(),
    mEvent(""),
    mAcceptList(),
    mSessionExpires(0),
    mMinSE(0),
    mSupportedList(),
    mSessionDescription(NULL),
    mBodyContent(""),
    mContentType(""),
    mIsRequest(false),
    //mDialogID(""),
    mRequestMethod(SIP_METHOD_UNKNOWN),
    mRequestURI(""),
    mResponseCode(-1),
    mResponseReason(""),
    mRefresher(SIPSDK_UA_UNKNOWN)
{
    SIPSDK_CLASSTRACE("SDKSipMessage::SDKSipMessage");
    
    this->mFrom = new SipFromImpl;
    this->mFrom->AddRef();

    this->mTo = new SipToImpl;
    this->mTo->AddRef();

    this->mCSeq = new SipCSeqImpl;
    this->mCSeq->AddRef();

    this->mSessionDescription = new SessionDescriptionImpl();
    this->mSessionDescription->AddRef();
}

SDKSipMessage::~SDKSipMessage()
{
    SIPSDK_CLASSTRACE("SDKSipMessage::~SDKSipMessage");
    
    if(this->mFrom)
    {
        this->mFrom->ReleaseRef();
        this->mFrom = NULL;
    }

    if(this->mTo)
    {
        this->mTo->ReleaseRef();
        this->mTo = NULL;
    }

    if(this->mCSeq)
    {
        this->mCSeq->ReleaseRef();
        this->mCSeq = NULL;
    }
    
    if(this->mSessionDescription)
    {
        this->mSessionDescription->ReleaseRef();
        this->mSessionDescription = NULL;
    }
    
    ::DestroyContactList(this->mContactList);
    
    /*
    SIPSDK_SAFE_DELETE(mFrom);
    SIPSDK_SAFE_DELETE(mTo);
    SIPSDK_SAFE_DELETE(mCSeq);
    SIPSDK_SAFE_DELETE(mSessionDescription);
    */
}

void SDKSipMessage::AddRef()
{
    CReferenceControl::add_reference();
}

void SDKSipMessage::ReleaseRef()
{
    CReferenceControl::release_reference();
}

void SDKSipMessage::SetRefresher(SIPSDK_UA_Type t)
{
    mRefresher = t;
}

SIPSDK_UA_Type SDKSipMessage::GetRefresher() const
{
    return mRefresher;
}


string SDKSipMessage::GetAlso() const
{
    return this->mAlso;
}

void SDKSipMessage::SetAlso(string value)
{
    this->mAlso= value;
}

string SDKSipMessage::GetRefer() const
{
    return this->mRefer;
}

void SDKSipMessage::SetRefer(string value)
{
    this->mRefer= value;
}

CStrRef SDKSipMessage::GetCallID() const
{
    return this->mCallID;
}

void SDKSipMessage::SetCallID(string callId)
{
    this->mCallID = callId;
}

void SDKSipMessage::GetContactList(ContactList & result) const
{
    DestroyContactList(result);

    result.assign(this->mContactList.begin(), this->mContactList.end());
}

void SDKSipMessage::SetContactList(ContactList & contactList)
{
    DestroyContactList(this->mContactList);
    
    this->mContactList.assign(contactList.begin(), contactList.end());
    ContactList::iterator it = this->mContactList.begin();
    for(; it != this->mContactList.end(); ++ it)
        (*it)->AddRef();
}

void SDKSipMessage::GetSupportedList(StrContainer & result) const
{
    result.clear();
    result.assign(this->mSupportedList.begin(), this->mSupportedList.end());
}

void SDKSipMessage::SetSupportedList(const StrContainer & supportedList)
{
    this->mSupportedList.clear();
    this->mSupportedList.assign(supportedList.begin(), supportedList.end());
}

bool SDKSipMessage::IsContainSupportedTag(string tag)
{
    StrContainerIter iter = find(this->mSupportedList.begin(), this->mSupportedList.end(), tag);
    return iter != this->mSupportedList.end();
}

ISipCSeq* SDKSipMessage::GetCSeq() const
{
    return this->mCSeq;
}

ISipFrom* SDKSipMessage::GetFrom() const
{
    return this->mFrom;
}

ISipTo* SDKSipMessage::GetTo() const
{
    return this->mTo;
}

void SDKSipMessage::SetCSeq(ISipCSeq * cseq)
{
    SIPSDK_ASSERT_RETURN_VOID(cseq);    
        
    if(this->mCSeq) //SIPSDK_SAFE_DELETE(mCSeq);
        this->mCSeq->ReleaseRef();
    
    this->mCSeq = cseq;
    this->mCSeq->AddRef();
}
void SDKSipMessage::SetFrom(ISipFrom * from)
{
    SIPSDK_ASSERT_RETURN_VOID(from);

    if(this->mFrom) //SIPSDK_SAFE_DELETE(mFrom);
        this->mFrom->ReleaseRef();
    
    this->mFrom = from;
    this->mFrom->AddRef();
}
void SDKSipMessage::SetTo(ISipTo * to)
{
    SIPSDK_ASSERT_RETURN_VOID(to);

    if(this->mTo) //SIPSDK_SAFE_DELETE(mTo);
        this->mTo->ReleaseRef();
    
    this->mTo = to;
    this->mTo->AddRef();
}

void SDKSipMessage::GetViaList(StrContainer & result) const
{
    result.clear();
    result.assign(this->mViaList.begin(), this->mViaList.end());
}

void SDKSipMessage::SetViaList(const StrContainer & viaList)
{
    this->mViaList.clear();
    this->mViaList.assign(viaList.begin(), viaList.end());
}

void SDKSipMessage::GetAllowEventList(StrContainer & result) const
{
    result.clear();
    result.assign(this->mAllowEventList.begin(), this->mAllowEventList.end());
}

void SDKSipMessage::SetAllowEventList(const StrContainer & allowEventList)
{
    this->mAllowEventList.clear();
    this->mAllowEventList.assign(allowEventList.begin(), allowEventList.end());
}

void SDKSipMessage::GetRequireList(StrContainer & result) const
{
    result.clear();
    result.assign(this->mRequireList.begin(), this->mRequireList.end());
}

void SDKSipMessage::SetRequireList(const StrContainer & requires)
{
    this->mRequireList.clear();
    this->mRequireList.assign(requires.begin(), requires.end());
}

CStrRef SDKSipMessage::GetEvent() const
{
    return this->mEvent;
}

void SDKSipMessage::SetEvent(string event)
{
    this->mEvent = event;
}

void SDKSipMessage::GetAcceptList(StrContainer & result) const
{
    result.clear();
    result.assign(this->mAcceptList.begin(), this->mAcceptList.end());
}

void SDKSipMessage::SetAcceptList(const StrContainer & acceptList)
{
    this->mAcceptList.clear();
    this->mAcceptList.assign(acceptList.begin(), acceptList.end());
}
    
uint32 SDKSipMessage::GetSessionExpires() const
{
    return this->mSessionExpires;
}
void SDKSipMessage::SetSessionExpires(uint32 sessionExpires)
{
    this->mSessionExpires = sessionExpires;
}


uint32 SDKSipMessage::GetMinSE() const
{
    return this->mMinSE;
}

void SDKSipMessage::SetMinSE(uint32 minSE)
{
    this->mMinSE = minSE;
}
    
ISessionDescription* SDKSipMessage::GetSessionDescription() const
{
    return this->mSessionDescription;
}

void SDKSipMessage::SetSessionDescription(ISessionDescription * sdp)
{
    SIPSDK_ASSERT_RETURN_VOID(sdp);

    SIPSDK_SAFE_RELEASE(mSessionDescription);
    
    this->mSessionDescription = sdp;
    SIPSDK_SAFE_ADDREF(mSessionDescription);

    this->SetContentType("application/sdp");
}

CStrRef SDKSipMessage::GetBodyContent() const
{
    return this->mBodyContent;
}

void SDKSipMessage::SetBodyContent(string content)
{
    this->mBodyContent.assign(content);
}

CStrRef SDKSipMessage::GetContentType() const
{
    return this->mContentType;
}

void SDKSipMessage::SetContentType(string contentType)
{
    this->mContentType.assign(contentType);
}

bool SDKSipMessage::IsSipRequest() const
{
    return this->mIsRequest;
}

void SDKSipMessage::SetIsSipRequest(bool flag)
{
    this->mIsRequest = flag;
}

//CStrRef SDKSipMessage::GetDialogID()
//{
//    return this->mDialogID;
//}

SipMethod SDKSipMessage::GetRequstMethod() const
{
    return this->mRequestMethod;
}

void SDKSipMessage::SetRequstMethod(SipMethod method)
{
    this->mRequestMethod = method;
}

CStrRef SDKSipMessage::GetRequestURI() const
{
    return this->mRequestURI;
}

void SDKSipMessage::SetRequestURI(string URI)
{
    this->mRequestURI = URI;
}

int SDKSipMessage::GetResponseStatusCode() const
{
    return this->mResponseCode;
}

void SDKSipMessage::SetResponseStatusCode(int code)
{
    this->mResponseCode = code;
}

CStrRef SDKSipMessage::GetResponseReason() const
{
    return this->mResponseReason;
}

void SDKSipMessage::SetResponseReason(string reason)
{
    this->mResponseReason = reason;
}

string SDKSipMessage::toString() const
{
    stringstream msg;
    msg << "sip message [" << (mIsRequest ? "request(" : "response(");

    if(mIsRequest)
        msg << mRequestURI << " " << ::ConvertMethod2Str(mRequestMethod) << ")";
    else
        msg << mResponseCode << " " << mResponseReason << ")";
    
    msg << ", Uri: " << mRequestURI;
    msg << ", From: " << mFrom->toString();
    msg << ", To: " << mTo->toString();
    msg << ", Call-ID: " << mCallID;
    msg << ", CSeq: " << mCSeq->toString();
    msg << "]";

    return msg.str();
}

bool SDKSipMessage::IsContainSdp() const
{
    return this->mContentType.compare("application/sdp") == 0;
}

string SDKSipMessage::GetLocalTag()
{
    // receive request from remote, then local tag is in "Sip To"
    // receive response from remote, then local tag is in "Sip From"
    if(this->IsSipRequest())
        return this->mTo->GetTag();
    else
        return this->mFrom->GetTag();
}
string SDKSipMessage::GetRemoteTag()
{
    // receive request from remote, then remote tag is in "Sip From"
    // receive response from remote, then remote tag is in "Sip to"
    if(this->IsSipRequest())
        return this->mFrom->GetTag();
    else
        return this->mTo->GetTag();
}


static map<SipMethod, string> gSipMethod2StrMap;
static map<string, SipMethod> gSipStr2MethodMap;
    
void InitMethodCfg()
{
    // Added by Sam to avoid init repeatly
    static bool flag = false;
    
    if(flag)
        return;

    flag = true;
    // End add
    
    gSipMethod2StrMap[SIP_METHOD_INVITE]    = "INVITE";
    gSipMethod2StrMap[SIP_METHOD_ACK]       = "ACK";
    gSipMethod2StrMap[SIP_METHOD_OPTIONS]   = "OPTIONS";
    gSipMethod2StrMap[SIP_METHOD_BYE]       = "BYE";
    gSipMethod2StrMap[SIP_METHOD_CANCEL]    = "CANCEL";
    gSipMethod2StrMap[SIP_METHOD_REGISTER]  = "REGISTER";
    gSipMethod2StrMap[SIP_METHOD_INFO]      = "INFO";
    gSipMethod2StrMap[SIP_METHOD_UPDATE]    = "UPDATE";
    gSipMethod2StrMap[SIP_METHOD_TRANSFER]  = "TRANSFER";
    gSipMethod2StrMap[SIP_METHOD_REFER]     = "REFER";
    gSipMethod2StrMap[SIP_METHOD_SUBSCRIBE] = "SUBSCRIBE";
    gSipMethod2StrMap[SIP_METHOD_NOTIFY]    = "NOTIFY";
    gSipMethod2StrMap[SIP_METHOD_MESSAGE]   = "MESSAGE";
    gSipMethod2StrMap[SIP_METHOD_UNKNOWN]   = "UNKNOWN";

    gSipStr2MethodMap["INVITE"]    = SIP_METHOD_INVITE;
    gSipStr2MethodMap["ACK"]       = SIP_METHOD_ACK;
    gSipStr2MethodMap["OPTIONS"]   = SIP_METHOD_OPTIONS;
    gSipStr2MethodMap["BYE"]       = SIP_METHOD_BYE;
    gSipStr2MethodMap["CANCEL"]    = SIP_METHOD_CANCEL;
    gSipStr2MethodMap["REGISTER"]  = SIP_METHOD_REGISTER;
    gSipStr2MethodMap["INFO"]      = SIP_METHOD_INFO;
    gSipStr2MethodMap["UPDATE"]    = SIP_METHOD_UPDATE;
    gSipStr2MethodMap["TRANSFER"]  = SIP_METHOD_TRANSFER;
    gSipStr2MethodMap["REFER"]     = SIP_METHOD_REFER;
    gSipStr2MethodMap["SUBSCRIBE"] = SIP_METHOD_SUBSCRIBE;
    gSipStr2MethodMap["NOTIFY"]    = SIP_METHOD_NOTIFY;
    gSipStr2MethodMap["MESSAGE"]   = SIP_METHOD_MESSAGE;
    gSipStr2MethodMap["UNKNOWN"]   = SIP_METHOD_UNKNOWN;
    
}

string ConvertMethod2Str(SipMethod method)
{
    InitMethodCfg();
    
    map<SipMethod, string>::iterator iter = gSipMethod2StrMap.find(method);
    if(iter == gSipMethod2StrMap.end())
        return "UNKNOWN";

    string strMethod = iter->second;
    return strMethod;
}

SipMethod ConvertStr2Method(CStrRef str)
{
    InitMethodCfg();
    
    map<string, SipMethod>::iterator iter = gSipStr2MethodMap.find(str);
    if(iter == gSipStr2MethodMap.end())
        return SIP_METHOD_UNKNOWN;

    SipMethod method = iter->second;
    return method;
}
