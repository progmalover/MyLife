#ifndef _SIP_SDK_RESIP_STACK_ADAPTER_IMPL_H__
#define _SIP_SDK_RESIP_STACK_ADAPTER_IMPL_H__

#include "SipStackAdapter.h"
#include "SipMessageImpl.h"
#include "MediaDescriptionImpl.h"
#include "SipUtil.h"

class SessionDescriptionImpl;

class ResipSipMessage : public SDKSipMessage
{
public:
    friend class SessionDescriptionImpl;

    ResipSipMessage();
    ResipSipMessage(SipMessage * msg);
    virtual ~ResipSipMessage();

	virtual void    AddRef();
	virtual void    ReleaseRef();

    CmResult        Sync2SDK();
    SipMessage *    Sync2Resip();
    
    //SipMessage *    Sync2NewResip();
    SipMessage *    GetResipMessage() const { return mResipMessage; }
    static string   EncodeResipMsg(SipMessage * msg);
    
private:
    CmResult        FillHeader2SDK();
    CmResult            FillAlso2SDK();
    CmResult            FillRefer2SDK();
    CmResult            FillCallID2SDK();
    CmResult            FillContact2SDK();
    CmResult            FillContentType2SDK();
    CmResult            FillVias2SDK();
    CmResult            FillAllowEvents2SDK();
    CmResult            FillRequire2SDK();
    CmResult            FillEvent2SDK();
    CmResult            FillAccepts2SDK();
    CmResult            FillSessionExpires2SDK();
    CmResult            FillSupported2SDK();
    CmResult            FillCSeq2SDK();
    CmResult            FillFrom2SDK();
    CmResult            FillTo2SDK();
    CmResult            FillRequsetLine2SDK();
    CmResult            FillResponseLine2SDK();
    CmResult        FillSdp2SDK();
    CmResult            FillSdpBase2SDK(SdpContents* sdp, SessionDescriptionImpl * sdkSDP);
    CmResult                FillSdpOrigin2SDK(SdpContents* sdp, SessionDescriptionImpl * sdkSDP);
    CmResult                FillSdpConn2SDK(SdpContents* sdp, SessionDescriptionImpl * sdkSDP);
    CmResult                FillSdpEmail2SDK(SdpContents* sdp, SessionDescriptionImpl * sdkSDP);
    CmResult                FillSdpPhone2SDK(SdpContents* sdp, SessionDescriptionImpl * sdkSDP);
    CmResult                FillSdpTime2SDK(SdpContents* sdp, SessionDescriptionImpl * sdkSDP);
    CmResult            FillSdpMedia2SDK(SdpContents* sdp, ISessionDescription::MediaDescrContainer& sdkMediaDescList);
    CmResult                FillSdpMediaConn2SDK(SDMediaDescriptionImpl* sdkMediaDesc, const SdpContents::Session::Medium& media);
    CmResult                FillSdpMediaMedium2SDK(SDMediaDescriptionImpl* sdkMediaDesc, const SdpContents::Session::Medium& media);
    CmResult                FillSdpMediaCodec2SDK(SDMediaDescriptionImpl* sdkMediaDesc, const SdpContents::Session::Medium& media);
    CmResult                FillSdpMediaCrypto2SDK(SDMediaDescriptionImpl* sdkMediaDesc, const SdpContents::Session::Medium& media);

    CmResult        FillHeader2Resip(SipMessage* resipMsg);
    CmResult            FillAlso2Resip(SipMessage* resipMsg);
    CmResult            FillRefer2Resip(SipMessage* resipMsg);
    CmResult            FillCallID2Resip(SipMessage* resipMsg);
    CmResult            FillContact2Resip(SipMessage* resipMsg);
    CmResult            FillContentType2Resip(SipMessage* resipMsg);
    CmResult            FillVias2Resip(SipMessage* resipMsg);
    CmResult            FillAllowEvents2Resip(SipMessage* resipMsg);
    CmResult            FillRequire2Resip(SipMessage* resipMsg);
    CmResult            FillEvent2Resip(SipMessage* resipMsg);
    CmResult            FillAccepts2Resip(SipMessage* resipMsg);
    CmResult            FillSessionExpires2Resip(SipMessage* resipMsg);
    CmResult            FillSupported2Resip(SipMessage* resipMsg);
    CmResult            FillCSeq2Resip(SipMessage* resipMsg);
    CmResult            FillFrom2Resip(SipMessage* resipMsg);
    CmResult            FillTo2Resip(SipMessage* resipMsg);
    CmResult            FillRequsetLine2Resip(SipMessage* resipMsg);
    CmResult            FillResponseLine2Resip(SipMessage* resipMsg);
    
    //CmResult        FillSdpBase2Resip();
    //CmResult        FillSdpMedia2Resip();

    
    //void            GenRqstStartLine(string& startLine);
    //void            GenRespStatusLine(string& statusLine);

private:
    SipMessage *    mResipMessage;
    int             mLastRespCode;
};

class ResipStat : public StackStat, CReferenceControl
{
public:
    ResipStat();
    virtual ~ResipStat();

	virtual void    AddRef();
	virtual void    ReleaseRef();
    
    virtual uint32  GetValue(StackStat::StatType type) const;
    virtual uint32  Get2xxInSum(SipMethod method) const;
    virtual uint32  GetErrInSum(SipMethod method) const;
    virtual uint32  Get2xxOutSum(SipMethod method) const;
    virtual uint32  GetErrOutSum(SipMethod method) const;

    StatisticsMessage::Payload& GetPayLoad();
    
private:
    StatisticsMessage::Payload mPayLoad;
};

class ResipStatsHandler : public ExternalStatsHandler
{
public:
    ResipStatsHandler();
    virtual ~ResipStatsHandler();

    virtual bool operator()(StatisticsMessage &statsMessage);

    void SetResipStat(ResipStat * stat);
private:
    ResipStat* mResipStat;
};

class ResipStackAdapterImpl : public SipStackAdapter,
                              public CReferenceControl
{
public:
    ResipStackAdapterImpl();
    virtual ~ResipStackAdapterImpl();
    
	virtual void    AddRef();
	virtual void    ReleaseRef();
    
    virtual int     SetOpt(SipOption opt, void * s, size_t n);

    virtual void    SetTlsCertPath(string path);
    virtual void    SetTlsDomain(string domain);
    
    virtual CmResult AddTransport(
                        TransportProtocol protocol,
                        CStrRef           ipAddress, 
                        int               port, 
                        AddressType       type,
                        QosType           dscpValue);

    virtual bool    IsUsingTLS() const;
    
    // init sip stack
    virtual CmResult Init();

    // init sip stack log trace
    virtual CmResult InitLog(CStrRef path, CStrRef filePrefix, StackLogLevel level);
    
    //virtual CmResult Run();
    virtual CmResult Shutdown();
    //virtual CmResult Join();
    
    virtual CmResult        SendMsg(SipMessage * resipMsg);
    virtual ISipMessage *   RecvMsg();
    virtual StackStat*      GetStackStat();
    virtual int             GetTransportPort();
    virtual CStrRef         GetTransportName();
    
private:
    resip::Log::Level ConvertLogLevel(StackLogLevel level);
    int                    SetQoS(void * s, size_t n);
    
private:
    bool        mRunFlag;
    SipStack*   mStack;
    FdSet       mFdSet; 
    //bool        mNoMsgPending;
    bool        mHasInit;
    bool        mHasAddTransport;

    ResipStatsHandler*  mStackStatHandler;
    ResipStat*          mStackStat;
    string      mCertPath;
    string      mTlsDomain;
    bool        mIsUsingTLS;

    int         mTransportPort;
    string      mTransportName;

    FdPollGrp*  mPollGrp;
};

class ResipHelper
{
public:
    static ISDCrypto* parseCryptoLine(const Data& cryptoLine);
    static void       afterSocketCreation(Socket s, int transportType, const char* file, int line);
   
    static QosType    gDSCPValue;
    
private:
   static void parseCryptoParams(resip::ParseBuffer& pb, 
                                 string& keyMethod, 
                                 resip::Data& keyValue, 
                                 unsigned int& srtpLifetime, 
                                 unsigned int& srtpMkiValue, 
                                 unsigned int& srtpMkiLength);

};

extern "C"
{
    SipMethod    ConvertResipMethod2SDKMethod(MethodTypes method);
    MethodTypes  ConvertSDKMethod2ResipMethod(SipMethod method);
}

#endif

