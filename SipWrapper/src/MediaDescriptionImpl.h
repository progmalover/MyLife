#ifndef _SIP_SDK_MEDIA_DESCRIPTION_IMPL_H__
#define _SIP_SDK_MEDIA_DESCRIPTION_IMPL_H__

#include "sipsdk.h"
#include "AttributeImpl.h"
#include "SipUtil.h"

class SDCodecImpl: public ISDCodec
{
public:
    SDCodecImpl();
    SDCodecImpl(const SDCodecImpl& codec);
    virtual ~SDCodecImpl();

    SDCodecImpl& operator=(const SDCodecImpl& rhs);
    SDCodecImpl* Clone() const;
    
    //virtual void    AddRef();
    //virtual void    ReleaseRef();

    virtual CStrRef GetName() const;
    virtual void    SetName(string name);

    virtual uint32  GetRate() const;
    virtual void    SetRate(uint32 rate);

    virtual int     GetPayloadType() const;
    virtual void    SetPayloadType(int payloadType);

    virtual CStrRef GetParameters() const;
    virtual void    SetParameters(string parameters);

private:
    string  mName;
    uint32  mRate;
    int     mPayloadType;
    string  mParameters;
};

class SDCryptoParamImpl: public ISDCryptoParam
{
public:
    SDCryptoParamImpl();
    SDCryptoParamImpl(const SDCryptoParamImpl& param);
    virtual ~SDCryptoParamImpl();

    SDCryptoParamImpl& operator=(const SDCryptoParamImpl& rhs);
    SDCryptoParamImpl* Clone() const;
    
    CStrRef GetBase64Key() const;
    void    SetBase64Key(string base64Key);
    void    EncodeBase64Key();
    void    DecodeBase64Key();
    
    CStrRef GetMasterKey() const;
    void    SetMasterKey(string masterKey);
    
    CStrRef GetMasterSalt() const;
    void    SetMasterSalt(string masterSalt);
    
    uint32  GetLifeTime() const;
    string  GetLifeTimeStr();
    void    SetLifeTime(uint32 lifeTime);
    
    uint32  GetMasterKeyIndex() const;
    void    SetMasterKeyIndex(uint32 index);
    
    uint32  GetMasterKeyIndexLen() const;
    void    SetMasterKeyIndexLen(uint32 len);

    string  toString();

private:
    string  mBase64Key;
    string  mMasterKey;
    string  mMasterSalt;
    uint32  mLifeTime;
    uint32  mMasterKeyIndex;
    uint32  mMasterKeyIndexLen;
};

class SDCryptoImpl: public ISDCrypto
{
public:
    SDCryptoImpl();
    virtual ~SDCryptoImpl();
    SDCryptoImpl(const SDCryptoImpl& other);

    SDCryptoImpl& operator=(const SDCryptoImpl& rhs);
    SDCryptoImpl* Clone() const;
    
    uint32  GetTag() const;
    void    SetTag(uint32 tag);

    CStrRef GetCryptoSuiteType() const;
    void    SetCryptoSuiteType(string type);

    vector<ISDCryptoParam*>& GetAllParams();

private:
    uint32                  mTag;
    string                  mCryptoSuiteType;
    vector<ISDCryptoParam*> mParams;
};

class SDMediaDescriptionImpl: public ISDMediaDescription
{
public:
    SDMediaDescriptionImpl();
    SDMediaDescriptionImpl(const SDMediaDescriptionImpl & mediaDesc);
    virtual ~SDMediaDescriptionImpl();

    SDMediaDescriptionImpl& operator=(const SDMediaDescriptionImpl& rhs) ;
    SDMediaDescriptionImpl* Clone() const;
    //virtual void            AddRef();
    //virtual void            ReleaseRef();
    
    //virtual ISDMedia*       GetMedia() const;
    //virtual void            SetMedia(ISDMedia* media);

    virtual CStrRef                 GetMediaType() const;
    virtual void                    SetMediaType(string mediaType);

    virtual int                     GetMediaPort() const;
    virtual void                    SetMediaPort(int mediaPort);

    virtual CStrRef                 GetProtocol() const;
    virtual void                    SetProtocol(string protocol);
    
    // get attribute by payload
    virtual vector<int>&            GetPayLoadTypeList();
    
    virtual CStrRef                 GetInfo() const;
    virtual void                    SetInfo(string info);

    virtual vector<ISDConnection*>& GetConnectionList();    
    virtual ISDAttribute*           GetAttribute() const;
    virtual vector<ISDCodec*>&      GetAllCodecs();
    virtual vector<ISDCrypto*>&     GetAllCryptos();
    virtual void                    ClearAllCryptos();

    string                          toString();

private:
    //ISDMedia*               mMedia;
    int                     mMediaPort;
    string                  mMediaType;
    string                  mProtocal;
    vector<int>             mFormat;
    string                  mInfo;
    vector<ISDConnection*>  mConnectionList;
    SDAttributeImpl*        mAttribute;
    vector<ISDCodec*>       mCodecs;
    vector<ISDCrypto*>      mCryptos;
};

#endif
