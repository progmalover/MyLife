#ifndef _SIP_SDK_CONNECTION_IMPL_H__
#define _SIP_SDK_CONNECTION_IMPL_H__

#include "sipsdk.h"
#include "SipUtil.h"

class SDConnectionImpl: public ISDConnection,
                        public CReferenceControl
{
public:
    SDConnectionImpl();
    SDConnectionImpl(const SDConnectionImpl & conn);
    virtual ~SDConnectionImpl();

    SDConnectionImpl& operator=(const SDConnectionImpl& rhs);
    SDConnectionImpl* Clone() const;
    
    virtual void            AddRef();
    virtual void            ReleaseRef();
    
    virtual CStrRef         GetNetType() const;
    virtual void            SetNetType(string netType);
    virtual CStrRef         GetAddressType() const;
    virtual void            SetAddressType(string addressType);
    virtual CStrRef         GetAddress() const;
    virtual void            SetAddress(string address);
    
private:
    string  mNetType;
    string  mAddressType;
    string  mAddress;
};

#endif
