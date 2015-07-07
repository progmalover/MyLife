#include "ConnectionImpl.h"
#include "SipTrace.h"

/////////////////////////////////////////////////////////////////////
////SDConnectionImpl
ISDConnection* ISDConnection::Create()
{
    return new SDConnectionImpl();
}

SDConnectionImpl::SDConnectionImpl() :
    mNetType("IN"), mAddressType("IP4"), mAddress("")
{
    SIPSDK_CLASSTRACE("SDConnectionImpl::SDConnectionImpl");
}

SDConnectionImpl::SDConnectionImpl(const SDConnectionImpl & conn) :
    mNetType("IN"), mAddressType("IP4"), mAddress("")
{
    SIPSDK_CLASSTRACE("SDConnectionImpl::SDConnectionImpl");
    *this = conn;
}

SDConnectionImpl::~SDConnectionImpl()
{ 
    SIPSDK_CLASSTRACE("SDConnectionImpl::~SDConnectionImpl");
}

SDConnectionImpl& SDConnectionImpl::operator=(const SDConnectionImpl& rhs)
{
    if(this == &rhs)
        return *this;

    this->mNetType = rhs.mNetType;
    this->mAddressType = rhs.mAddressType;
    this->mAddress = rhs.mAddress;
        
    return *this;
}

SDConnectionImpl* SDConnectionImpl::Clone() const
{
    return new SDConnectionImpl(*this);
}

void SDConnectionImpl::AddRef()
{
    CReferenceControl::add_reference();
}

void SDConnectionImpl::ReleaseRef()
{
    CReferenceControl::release_reference();
}

CStrRef SDConnectionImpl::GetNetType() const
{
    return this->mNetType;
}

void SDConnectionImpl::SetNetType(string netType)
{
    this->mNetType = netType;
}

CStrRef SDConnectionImpl::GetAddressType() const
{
    return this->mAddressType;
}

void SDConnectionImpl::SetAddressType(string addressType)
{
    this->mAddressType = addressType;
}

CStrRef SDConnectionImpl::GetAddress() const
{
    return this->mAddress;
}

void SDConnectionImpl::SetAddress(string address)
{
    this->mAddress = address;
}

