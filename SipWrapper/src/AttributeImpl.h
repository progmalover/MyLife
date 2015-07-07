#ifndef _SIP_SDK_ATTRIBUTE_IMPL_H__
#define _SIP_SDK_ATTRIBUTE_IMPL_H__

#include "sipsdk.h"
#include "SipUtil.h"

class SDAttributeImpl: public ISDAttribute,
                       public CReferenceControl
{
public:
    SDAttributeImpl();
    virtual ~SDAttributeImpl();

    virtual void            AddRef();
    virtual void            ReleaseRef();
    
    virtual bool            IsKeyExist(CStrRef key) const;
    virtual void            GetValues(CStrRef key, StrContainer& result) const;
    
    virtual void            AddAttribute(CStrRef key, CStrRef value);
    virtual void            ClearAttribute(CStrRef key);

    virtual void                SetAttributeHelp(AttributeHelper&);
    virtual AttributeHelper&    GetAttributeHelp() { return *mAttributeHelp; }
private:
    AttributeHelper *       mAttributeHelp;
};

#endif
