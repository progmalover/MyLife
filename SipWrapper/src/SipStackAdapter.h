/*------------------------------------------------------*/
/* Initialize sip stack and let stack run               */
/*                                                      */
/* SipStackAdapter.h              `                     */
/*                                                      */
/* Copyright (C) WebEx Communications Inc.              */
/* All rights reserved                                  */
/*                                                      */
/*------------------------------------------------------*/

#ifndef _SIP_SDK_STACK_ADAPTER_IMPL_H__
#define _SIP_SDK_STACK_ADAPTER_IMPL_H__

#include "sipsdk.h"
#include "SipUtil.h"


class SipStackAdapter
{
public:
    virtual void            AddRef() = 0;
    virtual void            ReleaseRef() = 0;
    
    virtual void            SetTlsCertPath(string path) = 0;
    virtual void            SetTlsDomain(string domain) = 0;

    virtual CmResult        AddTransport(
                                TransportProtocol protocol,
                                CStrRef           ipAddress, 
                                int               port, 
                                AddressType       type,
                                QosType           dscpValue) = 0;

    virtual bool            IsUsingTLS() const = 0;
    
    virtual CmResult        Init(/* ... */) = 0;
    virtual CmResult        InitLog(CStrRef path, CStrRef filePrefix, StackLogLevel level) = 0;
    //virtual CmResult Run(/* ... */) = 0;
    virtual CmResult        Shutdown(/* ... */) = 0;
    //virtual CmResult Join(/* ... */) = 0;
    
    virtual CmResult        SendMsg(SipMessage * resipMsg) = 0;
    virtual ISipMessage *   RecvMsg() = 0;

    virtual StackStat*      GetStackStat() = 0;
    virtual int             GetTransportPort() = 0;
    virtual CStrRef         GetTransportName() = 0;

    // set sip SDK options
    virtual int SetOpt(SipOption opt, void * s, size_t n) = 0;
    
    static SipStackAdapter * CreateResipStackAdapter();
};

#endif
