#include "MediaDescriptionImpl.h"
#include "ConnectionImpl.h"
#include "SipTrace.h"
#include <sstream>
using std::stringstream;

/////////////////////////////////////////////////////////////////////
////SDMediaImpl
ISDCodec* ISDCodec::Create()
{
    return new SDCodecImpl();
}

SDCodecImpl::SDCodecImpl() :
    mName(""),
    mRate(8000),
    mPayloadType(-1),
    mParameters("")
{
    SIPSDK_CLASSTRACE("SDCodecImpl::SDCodecImpl");
}

SDCodecImpl::SDCodecImpl(const SDCodecImpl& codec) :
    mName(""),
    mRate(8000),
    mPayloadType(-1),
    mParameters("")
{
    SIPSDK_CLASSTRACE("SDCodecImpl::SDCodecImpl");
    *this = codec;
}

SDCodecImpl::~SDCodecImpl()
{ 
    SIPSDK_CLASSTRACE("SDCodecImpl::~SDCodecImpl");
}

SDCodecImpl& SDCodecImpl::operator=(const SDCodecImpl& rhs)
{
    if(this == &rhs)
        return *this;

    this->mName = rhs.mName;
    this->mRate = rhs.mRate;
    this->mPayloadType = rhs.mPayloadType;
    this->mParameters = rhs.mParameters;
    
    return *this;
}

SDCodecImpl* SDCodecImpl::Clone() const
{   
    return new SDCodecImpl(*this);
}

//void SDCodecImpl::AddRef()
//{
//    CReferenceControl::add_reference();
//}
//
//void SDCodecImpl::ReleaseRef()
//{
//    CReferenceControl::release_reference();
//}

CStrRef SDCodecImpl::GetName() const
{
    return this->mName;
}
void SDCodecImpl::SetName(string name)
{
    this->mName = name;
}

uint32 SDCodecImpl::GetRate() const
{
    return this->mRate;
}
void SDCodecImpl::SetRate(uint32 rate)
{
    this->mRate = rate;
}

int SDCodecImpl::GetPayloadType() const
{
    return this->mPayloadType;
}
void SDCodecImpl::SetPayloadType(int payloadType)
{
    this->mPayloadType = payloadType;
}

CStrRef SDCodecImpl::GetParameters() const
{
    return this->mParameters;
}
void SDCodecImpl::SetParameters(string parameters)
{
    this->mParameters = parameters;
}

/////////////////////////////////////////////////////////////////////
////SDCryptoParamImpl
ISDCryptoParam* ISDCryptoParam::Create()
{
    return new SDCryptoParamImpl();
}

SDCryptoParamImpl::SDCryptoParamImpl() :
    mBase64Key(""),
    mMasterKey(""),
    mMasterSalt(""),
    mLifeTime(0),
    mMasterKeyIndex(0),
    mMasterKeyIndexLen(0)
{
    SIPSDK_CLASSTRACE("SDCryptoParamImpl::SDCryptoParamImpl");
}

SDCryptoParamImpl::SDCryptoParamImpl(const SDCryptoParamImpl& param) :
    mBase64Key(""),
    mMasterKey(""),
    mMasterSalt(""),
    mLifeTime(0),
    mMasterKeyIndex(0),
    mMasterKeyIndexLen(0)
{
    SIPSDK_CLASSTRACE("SDCryptoParamImpl::SDCryptoParamImpl");
    *this = param;
}

SDCryptoParamImpl::~SDCryptoParamImpl()
{ 
    SIPSDK_CLASSTRACE("SDCryptoParamImpl::~SDCryptoParamImpl");
}

SDCryptoParamImpl& SDCryptoParamImpl::operator=(const SDCryptoParamImpl& rhs)
{
    if(this == &rhs)
        return *this;

    this->mBase64Key = rhs.mBase64Key;
    this->mMasterKey = rhs.mMasterKey;
    this->mMasterSalt = rhs.mMasterSalt;
    this->mLifeTime = rhs.mLifeTime;
    this->mMasterKeyIndex = rhs.mMasterKeyIndex;
    this->mMasterKeyIndexLen = rhs.mMasterKeyIndexLen;
    
    return *this;
}

SDCryptoParamImpl* SDCryptoParamImpl::Clone() const
{   
    return new SDCryptoParamImpl(*this);
}

CStrRef SDCryptoParamImpl::GetBase64Key() const
{
    return this->mBase64Key;
}

void SDCryptoParamImpl::SetBase64Key(string base64Key)
{
    this->mBase64Key = base64Key;

    this->DecodeBase64Key();
}

void SDCryptoParamImpl::DecodeBase64Key()
{
    if(this->mBase64Key.empty())
    {
        this->mMasterKey = "";
        this->mMasterSalt = "";
        return;
    }

    Data data(mBase64Key);
    Data rawData = Base64Coder::decode(data);
    string rawStr = rawData.c_str();

    if(rawStr.size() != 30)
    {
        SIPSDK_ERRTRACE("invalid crypto key: " << rawStr);
        this->mMasterKey = "";
        this->mMasterSalt = "";
        return;
    }

    this->mMasterKey = rawStr.substr(0, 16);
    this->mMasterSalt = rawStr.substr(16);

    SIPSDK_DETAILTRACE("SDCryptoParamImpl::DecodeBase64Key, mMasterKey:" << mMasterKey << " mMasterSalt:" << mMasterSalt);
}

void SDCryptoParamImpl::EncodeBase64Key()
{
    if(this->mMasterKey.empty() || this->mMasterSalt.empty())
    {
        this->mBase64Key = "";
        return;
    }
    
    string key = this->mMasterKey + this->mMasterSalt;
    SIPSDK_DETAILTRACE("SDCryptoParamImpl::EncodeBase64Key, key:" << key);
    Data data(key);
    
    this->mBase64Key = Base64Coder::encode(data).c_str();
}


CStrRef SDCryptoParamImpl::GetMasterKey() const
{
    return this->mMasterKey;
}
void SDCryptoParamImpl::SetMasterKey(string masterKey)
{
    this->mMasterKey = masterKey;

    this->EncodeBase64Key();
}

CStrRef SDCryptoParamImpl::GetMasterSalt() const
{
    return this->mMasterSalt;
}
void SDCryptoParamImpl::SetMasterSalt(string masterSalt)
{
    this->mMasterSalt = masterSalt;

    this->EncodeBase64Key();
}

uint32 SDCryptoParamImpl::GetLifeTime() const
{
    return this->mLifeTime;
}

string SDCryptoParamImpl::GetLifeTimeStr()
{
    if(mLifeTime <= 0)
        return "";

    uint32 num = ::Log2(mLifeTime);

    char szNum[16] = {0};
    ::sprintf(szNum, "2^%d", num);
    
    return szNum;
}

void SDCryptoParamImpl::SetLifeTime(uint32 lifeTime)
{
    this->mLifeTime = lifeTime;
}

uint32 SDCryptoParamImpl::GetMasterKeyIndex() const
{
    return this->mMasterKeyIndex;
}
void SDCryptoParamImpl::SetMasterKeyIndex(uint32 index)
{
    this->mMasterKeyIndex = index;
}

uint32 SDCryptoParamImpl::GetMasterKeyIndexLen() const
{
    return this->mMasterKeyIndexLen;
}
void SDCryptoParamImpl::SetMasterKeyIndexLen(uint32 len)
{
    this->mMasterKeyIndexLen = len;
}

string SDCryptoParamImpl::toString()
{
    stringstream strMedia;
    
    strMedia << this->GetBase64Key();

    string lifeTime = this->GetLifeTimeStr();
    if(!lifeTime.empty())
        strMedia << "|" << lifeTime;

    if(this->GetMasterKeyIndex() != 0 && this->GetMasterKeyIndexLen() != 0)
        strMedia << "|" << this->GetMasterKeyIndex() << ":" << this->GetMasterKeyIndexLen();

    return strMedia.str();
}

/////////////////////////////////////////////////////////////////////
////SDCryptoImpl 
ISDCrypto* ISDCrypto::Create()
{
    return new SDCryptoImpl();
}

SDCryptoImpl::SDCryptoImpl() :
    mTag(0),
    mCryptoSuiteType(""),
    mParams()
{
    SIPSDK_DETAILTRACE("SDCryptoImpl::SDCryptoImpl, this: " << this);
}

SDCryptoImpl::~SDCryptoImpl()
{ 
    SIPSDK_DETAILTRACE("SDCryptoImpl::~SDCryptoImpl, this: " << this);
    for_each(this->mParams.begin(), this->mParams.end(), SIPSDKDeleteObj() );
}

SDCryptoImpl::SDCryptoImpl(const SDCryptoImpl& other) :
    mTag(0),
    mCryptoSuiteType(""),
    mParams()
{
    *this = other;
}

SDCryptoImpl& SDCryptoImpl::operator=(const SDCryptoImpl& rhs)
{
    if(this == &rhs)
        return *this;

    this->mTag = rhs.mTag;
    this->mCryptoSuiteType = rhs.mCryptoSuiteType;

    vector<ISDCryptoParam*>::const_iterator iter = rhs.mParams.begin();
    for(; iter != rhs.mParams.end(); ++ iter)
    {
        SDCryptoParamImpl* param = static_cast<SDCryptoParamImpl*>(*iter);
        SDCryptoParamImpl* newParam = param->Clone();
        this->mParams.push_back(newParam);
    }
    
    return *this;
}

SDCryptoImpl* SDCryptoImpl::Clone() const
{   
    return new SDCryptoImpl(*this);
}

uint32 SDCryptoImpl::GetTag() const
{
    return this->mTag;
}
void SDCryptoImpl::SetTag(uint32 tag)
{
    this->mTag = tag;
}

CStrRef SDCryptoImpl::GetCryptoSuiteType() const
{
    return this->mCryptoSuiteType;
}
void SDCryptoImpl::SetCryptoSuiteType(string type)
{
    this->mCryptoSuiteType = type;
}

vector<ISDCryptoParam*>& SDCryptoImpl::GetAllParams()
{
    return this->mParams;
}

/////////////////////////////////////////////////////////////////////
////SDMediaDescriptionImpl
ISDMediaDescription* ISDMediaDescription::Create()
{
    return new SDMediaDescriptionImpl();
}

SDMediaDescriptionImpl::SDMediaDescriptionImpl() :
    mMediaType(""), mProtocal(""), mMediaPort(0), mFormat(),
    mInfo(""), mConnectionList(), mAttribute(NULL), mCodecs(), mCryptos()
{
    SIPSDK_DETAILTRACE("SDMediaDescriptionImpl::SDMediaDescriptionImpl, this: " << this);
    mAttribute = new SDAttributeImpl;
    mAttribute->AddRef();
}

SDMediaDescriptionImpl::SDMediaDescriptionImpl(const SDMediaDescriptionImpl & mediaDesc) :
    mMediaType(""), mProtocal(""), mMediaPort(0), mFormat(),
    mInfo(""), mConnectionList(), mAttribute(NULL), mCodecs(), mCryptos()
{
    mAttribute = new SDAttributeImpl;
    mAttribute->AddRef();
    
    *this = mediaDesc;
}

SDMediaDescriptionImpl::~SDMediaDescriptionImpl()
{ 
    SIPSDK_DETAILTRACE("SDMediaDescriptionImpl::~SDMediaDescriptionImpl, this: " << this);
    for_each(this->mConnectionList.begin(), this->mConnectionList.end(), SIPSDKDeleteObj() );
    vector<ISDConnection*>().swap(this->mConnectionList);
    
    if(this->mAttribute)
    {
        this->mAttribute->ReleaseRef();
        this->mAttribute = NULL;
    }

    for_each(this->mCodecs.begin(), this->mCodecs.end(), SIPSDKDeleteObj() );
    vector<ISDCodec*>().swap(this->mCodecs);

    for_each(this->mCryptos.begin(), this->mCryptos.end(), SIPSDKDeleteObj() );
    vector<ISDCrypto*>().swap(this->mCryptos);
}

SDMediaDescriptionImpl& SDMediaDescriptionImpl::operator=(const SDMediaDescriptionImpl& rhs)
{
    if (this == &rhs)
        return *this;

    for_each(mConnectionList.begin(), mConnectionList.end(), SIPSDKDeleteObj() );
    vector<ISDConnection*>().swap(mConnectionList);

    for_each(mCodecs.begin(), mCodecs.end(), SIPSDKDeleteObj() );
    vector<ISDCodec*>().swap(mCodecs);

    for_each(this->mCryptos.begin(), this->mCryptos.end(), SIPSDKDeleteObj() );
    vector<ISDCrypto*>().swap(this->mCryptos);

    mMediaPort = rhs.mMediaPort;
    mMediaType = rhs.mMediaType;
    mProtocal = rhs.mProtocal;
    mInfo = rhs.mInfo;
    
    mFormat.assign(rhs.mFormat.begin(), rhs.mFormat.end());

    for(vector<ISDConnection*>::const_iterator iter = rhs.mConnectionList.begin(); 
                                               iter != rhs.mConnectionList.end();
                                               ++ iter)
    {
        SDConnectionImpl* conn = static_cast<SDConnectionImpl*>(*iter);
        SDConnectionImpl* newConn = conn->Clone();
        mConnectionList.push_back(newConn);
    }

    for(vector<ISDCodec*>::const_iterator iter = rhs.mCodecs.begin(); 
                                          iter != rhs.mCodecs.end();
                                          ++ iter)
    {
        SDCodecImpl* codec = static_cast<SDCodecImpl*>(*iter);
        SDCodecImpl* newCodec = codec->Clone();
        mCodecs.push_back(newCodec);
    }

    for(vector<ISDCrypto*>::const_iterator iter = rhs.mCryptos.begin(); 
                                           iter != rhs.mCryptos.end();
                                           ++ iter)
    {
        SDCryptoImpl* crypto = static_cast<SDCryptoImpl*>(*iter);
        SDCryptoImpl* newCrypto = crypto->Clone();
        mCryptos.push_back(newCrypto);
    }
                                         
    this->mAttribute->SetAttributeHelp(rhs.mAttribute->GetAttributeHelp());

    return *this;
}

SDMediaDescriptionImpl* SDMediaDescriptionImpl::Clone() const
{
    return new SDMediaDescriptionImpl(*this);
}

CStrRef SDMediaDescriptionImpl::GetMediaType() const
{
    return this->mMediaType;
}

void SDMediaDescriptionImpl::SetMediaType(string mediaType)
{
    this->mMediaType = mediaType;
}

int SDMediaDescriptionImpl::GetMediaPort() const
{
    return this->mMediaPort;
}

void SDMediaDescriptionImpl::SetMediaPort(int mediaPort)
{
    this->mMediaPort = mediaPort;
}

CStrRef SDMediaDescriptionImpl::GetProtocol() const
{
    return this->mProtocal;
}

void SDMediaDescriptionImpl::SetProtocol(string protocol)
{
    this->mProtocal = protocol;
}

vector<int>& SDMediaDescriptionImpl::GetPayLoadTypeList() 
{
    return this->mFormat;
}

CStrRef SDMediaDescriptionImpl::GetInfo() const
{
    return this->mInfo;
}

void SDMediaDescriptionImpl::SetInfo(string info)
{
    this->mInfo = info;
}

vector<ISDConnection*>& SDMediaDescriptionImpl::GetConnectionList()
{
    return this->mConnectionList;
}

ISDAttribute* SDMediaDescriptionImpl::GetAttribute() const
{
    return this->mAttribute;
}

vector<ISDCodec*>& SDMediaDescriptionImpl::GetAllCodecs()
{
    return this->mCodecs;
}

vector<ISDCrypto*>& SDMediaDescriptionImpl::GetAllCryptos()
{
    return this->mCryptos;
}

void SDMediaDescriptionImpl::ClearAllCryptos()
{
    for_each(this->mCryptos.begin(), this->mCryptos.end(), SIPSDKDeleteObj() );
    vector<ISDCrypto*>().swap(this->mCryptos);
}

string SDMediaDescriptionImpl::toString()
{
    stringstream strMedia;
    
    strMedia << "m=" << this->GetMediaType() << " " << this->GetMediaPort() 
        << " " << this->GetProtocol();
    
    for(int n = 0; n < mFormat.size(); ++ n)
    {
        strMedia << " " << mFormat[n];
    }
    strMedia << Symbols::CRLF;

    // Added by Yewei Gang, 2011-12-15 13:15:23
    for(vector<ISDConnection*>::iterator iter  = mConnectionList.begin();
			                             iter != mConnectionList.end();
			                          ++ iter)
    {
        ISDConnection* temp = static_cast<ISDConnection*>(*iter);
		if(temp && !temp->GetAddressType().empty())
			strMedia << "c=" << temp->GetNetType() << " " << temp->GetAddressType() << " " << temp->GetAddress() << Symbols::CRLF;
	}

    for (vector<ISDCodec*>::iterator iter = mCodecs.begin();
                                     iter != mCodecs.end();
                                     ++ iter) 
    {
        ISDCodec* codec = *iter;
        strMedia << "a=rtpmap:" << codec->GetPayloadType() << " " << codec->GetName() 
            << "/" << codec->GetRate() << Symbols::CRLF;

        if (!codec->GetParameters().empty())
            strMedia << "a=fmtp:" << codec->GetPayloadType() << " " << codec->GetParameters() << Symbols::CRLF;
    }

    for (vector<ISDCrypto*>::iterator iter = mCryptos.begin();
                                      iter != mCryptos.end();
                                      ++ iter) 
    {
        ISDCrypto* crypto = *iter;
        strMedia << "a=crypto:" << crypto->GetTag() << " " << crypto->GetCryptoSuiteType() << " ";

        bool firstInline = true;
        vector<ISDCryptoParam*>& params = crypto->GetAllParams();
        for (vector<ISDCryptoParam*>::iterator iter = params.begin();
                                               iter != params.end();
                                               ++ iter) 
        {
            SDCryptoParamImpl* param = static_cast<SDCryptoParamImpl*>(*iter);

            if(firstInline)
                strMedia << "inline:" << param->toString();
            else
                strMedia << ";inline:" << param->toString();

            firstInline = false;
        }

        strMedia << Symbols::CRLF;
    }

    // media attribute
    SDAttributeImpl* attPtr = static_cast<SDAttributeImpl*>(this->GetAttribute());
    attPtr->ClearAttribute("rtpmap");
    attPtr->ClearAttribute("fmtp");
    if(attPtr != NULL)
    {
        AttributeHelper attHelp = attPtr->GetAttributeHelp();
        std::ostringstream ss;
        attHelp.encode(ss);
        strMedia << ss.str();
    }
    
    return strMedia.str();
}



