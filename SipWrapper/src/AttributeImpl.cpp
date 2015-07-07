#include "AttributeImpl.h"
#include "SipTrace.h"

/////////////////////////////////////////////////////////////////////
////SDAttributeImpl

SDAttributeImpl::SDAttributeImpl() : mAttributeHelp(NULL)
{
    SIPSDK_CLASSTRACE("SDAttributeImpl::SDAttributeImpl");
    this->mAttributeHelp = new AttributeHelper();
}

SDAttributeImpl::~SDAttributeImpl()
{ 
    SIPSDK_CLASSTRACE("SDAttributeImpl::~SDAttributeImpl");
    SIPSDK_SAFE_DELETE(this->mAttributeHelp);
}

void SDAttributeImpl::AddRef()
{
    CReferenceControl::add_reference();
}

void SDAttributeImpl::ReleaseRef()
{
    CReferenceControl::release_reference();
}

bool SDAttributeImpl::IsKeyExist(CStrRef key) const
{
    Data dataKey(key);
    return this->mAttributeHelp->exists(dataKey);
}

void SDAttributeImpl::GetValues(CStrRef key, StrContainer& result) const
{
    Data dataKey(key);
    const list<Data>& values = this->mAttributeHelp->getValues(dataKey);

    result.clear();
    for(list<Data>::const_iterator iter = values.begin(); iter != values.end(); iter ++)
        result.push_back((*iter).c_str());
}

void SDAttributeImpl::AddAttribute(CStrRef key, CStrRef value)
{
    Data dataKey(key);
    Data dataValue(value);
    this->mAttributeHelp->addAttribute(dataKey, dataValue);
}

void SDAttributeImpl::ClearAttribute(CStrRef key)
{
    Data dataKey(key);
    this->mAttributeHelp->clearAttribute(dataKey);
}

void SDAttributeImpl::SetAttributeHelp(AttributeHelper& attrHelp)
{
    *this->mAttributeHelp = attrHelp;
}

