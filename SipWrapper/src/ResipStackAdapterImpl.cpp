#include "ResipStackAdapterImpl.h"
#include "AttributeImpl.h"
#include "ConnectionImpl.h"
#include "ConnectionImpl.h"
#include "MediaDescriptionImpl.h"
#include "SessionDescriptionImpl.h"
#include "SipTrace.h"
#include <sstream>
using std::stringstream;

UnknownHeaderType h_Also("Also");
UnknownHeaderType h_Refer("Refer");

/////////////////////////////////////////////////////////////////////
////ResipSipMessage
ISipMessage * ISipMessage::CreateSipRequest()
{
    ResipSipMessage* msg = new ResipSipMessage;
    msg->SetIsSipRequest(true);

    SIPSDK_INFOTRACE("ISipMessage::CreateSipRequest, request: " << msg);
    return msg;
}

ISipMessage * ISipMessage::CreateSipResponse(ISipMessage* request)
{
    ResipSipMessage* tmpRequest = dynamic_cast<ResipSipMessage*>(request);
    
    SIPSDK_ASSERT_RETURN(tmpRequest, NULL);
    SIPSDK_ASSERT_RETURN(tmpRequest->IsSipRequest(), NULL);
    SIPSDK_ASSERT_RETURN(tmpRequest->GetResipMessage(), NULL);
    
    SipMessage* resip = dynamic_cast<SipMessage*>(tmpRequest->GetResipMessage()->clone());
        
    ResipSipMessage* response = new ResipSipMessage(resip);
    response->SetIsSipRequest(false);

    // keep the from, to unchange with request in response
    SipFromImpl* from = new SipFromImpl(request->GetFrom()->toString());
    response->SetFrom(from);

    SipToImpl* to = new SipToImpl(request->GetTo()->toString());
    if(to->GetTag().empty())
    {
        Data dataTag = Helper::computeTag(Helper::tagSize);
        to->SetTag(dataTag.c_str());
    }
        
    response->SetTo(to);
    response->SetCallID(request->GetCallID());

    SipCSeqImpl* cseq = new SipCSeqImpl(request->GetCSeq()->toString());
    response->SetCSeq(cseq);
    
    SIPSDK_INFOTRACE("ISipMessage::CreateSipResponse(ISipMessage* request), response: " << response);
    return response;
}

ResipSipMessage::ResipSipMessage() : mResipMessage(NULL)
{
    SIPSDK_CLASSTRACE("ResipSipMessage::ResipSipMessage");
}

ResipSipMessage::ResipSipMessage(SipMessage * msg) : mResipMessage(msg), mLastRespCode(-1)
{
    SIPSDK_CLASSTRACE("ResipSipMessage::ResipSipMessage");
}

ResipSipMessage::~ResipSipMessage()
{
    SIPSDK_CLASSTRACE("ResipSipMessage::~ResipSipMessage");
    SIPSDK_SAFE_DELETE(mResipMessage);
}

void ResipSipMessage::AddRef()
{
    SDKSipMessage::AddRef();
}

void ResipSipMessage::ReleaseRef()
{
    SDKSipMessage::ReleaseRef();
}

CmResult ResipSipMessage::Sync2SDK()
{
    SIPSDK_INFOTRACE("ResipSipMessage::Sync2SDK, this: " << this);
    SIPSDK_ASSERT_RETURN(this->mResipMessage, SIPSDK_ERROR_NULL_POINTER);

    // fill the sip headers
    CmResult ret = this->FillHeader2SDK();
    SIPSDK_ASSERT_RETURN(SIPSDK_ERROR_NOERROR == ret, ret);

    // fill the sip body
    Contents* body = this->mResipMessage->getContents();
    if(body)
    {
        this->SetBodyContent(body->getBodyData().c_str());
    }
    /*
    const HeaderFieldValue* hfv = this->mResipMessage->getRawBody();
    Data sipMsgBody;
    hfv->toBorrowData(sipMsgBody);
    this->SetBodyContent(sipMsgBody.c_str());
    */
    
    // fill sdp
    ret = this->FillSdp2SDK();
    SIPSDK_ASSERT_RETURN(SIPSDK_ERROR_NOERROR == ret, ret);
    
    return SIPSDK_ERROR_NOERROR;
}

CmResult ResipSipMessage::FillHeader2SDK()
{
    SIPSDK_ASSERT_RETURN(this->mResipMessage, SIPSDK_ERROR_NULL_POINTER);
    
    this->FillAlso2SDK();
    this->FillRefer2SDK();
    this->FillCallID2SDK();
    this->FillContact2SDK();
    this->FillContentType2SDK();
    this->FillVias2SDK();
    this->FillAllowEvents2SDK();
    this->FillRequire2SDK();
    this->FillEvent2SDK();
    this->FillAccepts2SDK();
    this->FillSessionExpires2SDK();
    this->FillSupported2SDK();
    this->FillCSeq2SDK();
    this->FillFrom2SDK();
    this->FillTo2SDK();
    this->FillRequsetLine2SDK();
    this->FillResponseLine2SDK();

    return SIPSDK_ERROR_NOERROR;
}

CmResult ResipSipMessage::FillAlso2SDK()
{
    SIPSDK_ASSERT_RETURN(this->mResipMessage, SIPSDK_ERROR_NULL_POINTER);
    if(mResipMessage->exists(h_Also) && !mResipMessage->header(h_Also).empty())
    {
        Data also = this->mResipMessage->header(h_Also).front().value();
        
        this->SetAlso(also.c_str());
        SIPSDK_DETAILTRACE("ResipSipMessage::FillAlso2SDK, Also: " << also.c_str());
    }
    
    return SIPSDK_ERROR_NOERROR;
}

CmResult ResipSipMessage::FillRefer2SDK()
{
    SIPSDK_ASSERT_RETURN(this->mResipMessage, SIPSDK_ERROR_NULL_POINTER);
    if(mResipMessage->exists(h_Refer) && !mResipMessage->header(h_Refer).empty())
    {
        Data refer = this->mResipMessage->header(h_Refer).front().value();
        
        this->SetRefer(refer.c_str());
        SIPSDK_DETAILTRACE("ResipSipMessage::FillRefer2SDK, Refer: " << refer.c_str());
    }
    
    return SIPSDK_ERROR_NOERROR;
}

CmResult ResipSipMessage::FillCallID2SDK()
{
    SIPSDK_ASSERT_RETURN(this->mResipMessage, SIPSDK_ERROR_NULL_POINTER);
    
    if(mResipMessage->exists(h_CallId))
    {
        Data callID = this->mResipMessage->header(h_CallId).value();
        
        this->SetCallID(callID.c_str());
        SIPSDK_DETAILTRACE("ResipSipMessage::FillCallID2SDK, Call-ID: " << callID.c_str());
    }
    
    return SIPSDK_ERROR_NOERROR;
}

CmResult ResipSipMessage::FillContact2SDK()
{
    SIPSDK_ASSERT_RETURN(this->mResipMessage, SIPSDK_ERROR_NULL_POINTER);
    
    ContactList contactList;
    for (NameAddrs::iterator iter  = this->mResipMessage->header(h_Contacts).begin();
                             iter != this->mResipMessage->header(h_Contacts).end(); 
                             ++iter)
    {
        bool    has_expires = false;
        bool    has_q       = false;
        int     expires     = 0;
        float   q           = 1.0;
        Data    uri_data    = iter->uri().toString();
        
        string  uri(uri_data.c_str(), uri_data.
        		size());
        if((*iter).exists(p_expires))
        {
            has_expires = true;
            expires     = iter->param(p_expires);
        }
        
        if(iter->exists(p_q))
        {
            has_q = true;
            q   = iter->param(p_q);
        }
        
        LPCSTR display_name =  iter->displayName().c_str();

        SipContactImpl::AddNewContact(contactList, uri, display_name, has_expires, expires, has_q, q);
        SIPSDK_DETAILTRACE("ResipSipMessage::FillContact2SDK, Contact: " << uri);
    }
    
    this->SetContactList(contactList);
    DestroyContactList(contactList);

    return SIPSDK_ERROR_NOERROR;
}

CmResult ResipSipMessage::FillContentType2SDK()
{
    SIPSDK_ASSERT_RETURN(this->mResipMessage, SIPSDK_ERROR_NULL_POINTER);
    
    if(this->mResipMessage->exists(h_ContentType))
    {    
        const char* type = this->mResipMessage->header(h_ContentType).type().c_str();
        const char* subType = this->mResipMessage->header(h_ContentType).subType().c_str();
        string contentType;
        contentType.assign(type);
        contentType.append("/");
        contentType.append(subType);
        this->SetContentType(contentType);
        SIPSDK_DETAILTRACE("ResipSipMessage::FillContentType2SDK, Content-Type: " << this->GetContentType());
    }
    
    return SIPSDK_ERROR_NOERROR;
}

CmResult ResipSipMessage::FillVias2SDK()
{
    SIPSDK_ASSERT_RETURN(this->mResipMessage, SIPSDK_ERROR_NULL_POINTER);
    
    if(this->mResipMessage->exists(h_Vias))
    { 
        StrContainer viaList;
        for (Vias::iterator iter = this->mResipMessage->header(h_Vias).begin();
                            iter != this->mResipMessage->header(h_Vias).end(); 
                            ++iter)
        {
            stringstream ss;
            (*iter).encodeParsed(ss);
            viaList.push_back(ss.str());
            
            SIPSDK_DETAILTRACE("ResipSipMessage::FillVias2SDK, ss : "<< ss.str());
        }
        this->SetViaList(viaList);
    }

    return SIPSDK_ERROR_NOERROR;
}

CmResult ResipSipMessage::FillAllowEvents2SDK()
{
    SIPSDK_ASSERT_RETURN(this->mResipMessage, SIPSDK_ERROR_NULL_POINTER);
    
    if(this->mResipMessage->exists(h_AllowEvents))
    {
        StrContainer allowEventList;
        for (Tokens::iterator iter = this->mResipMessage->header(h_AllowEvents).begin();
                            iter != this->mResipMessage->header(h_AllowEvents).end(); 
                            ++iter)
        {
            Token token = (*iter);
            allowEventList.push_back(token.value().c_str());
            SIPSDK_DETAILTRACE("ResipSipMessage::FillAllowEvents2SDK, Allow-Events: " << token.value().c_str());
        }
        this->SetAllowEventList(allowEventList);
    }
    
    return SIPSDK_ERROR_NOERROR;
}

CmResult ResipSipMessage::FillRequire2SDK()
{
    SIPSDK_ASSERT_RETURN(this->mResipMessage, SIPSDK_ERROR_NULL_POINTER);
    
    if(this->mResipMessage->exists(h_Requires))
    {
        StrContainer requireList;
        for (Tokens::iterator iter  = this->mResipMessage->header(h_Requires).begin();
                              iter != this->mResipMessage->header(h_Requires).end(); 
                           ++ iter)
        {
            Token token = (*iter);
            Data  token_value = token.value();
            
            requireList.push_back(token_value.c_str());
            SIPSDK_DETAILTRACE("ResipSipMessage::FillRequire2SDK, Require: " << token_value.c_str());
        }
        
        this->SetRequireList(requireList);
    }
    
    return SIPSDK_ERROR_NOERROR;
}

CmResult ResipSipMessage::FillEvent2SDK()
{
    SIPSDK_ASSERT_RETURN(this->mResipMessage, SIPSDK_ERROR_NULL_POINTER);
    
    if (this->mResipMessage->exists(h_Event))
    {
        Data event_value = this->mResipMessage->header(h_Event).value();
        
        this->SetEvent(event_value.c_str());
        SIPSDK_DETAILTRACE("ResipSipMessage::FillEvent2SDK, Event: " << event_value.c_str());
    }
    
    return SIPSDK_ERROR_NOERROR;
}

CmResult ResipSipMessage::FillAccepts2SDK()
{
    SIPSDK_ASSERT_RETURN(this->mResipMessage, SIPSDK_ERROR_NULL_POINTER);
    
    if (this->mResipMessage->exists(h_Accepts))
    {
        StrContainer acceptList;
        for (Mimes::iterator iter = this->mResipMessage->header(h_Accepts).begin();
                            iter != this->mResipMessage->header(h_Accepts).end(); 
                            ++iter)
        {
            const char* type = (*iter).type().c_str();
            const char* subType = (*iter).subType().c_str();
            string accept;
            accept.assign(type);
            accept.append("/");
            accept.append(subType);
            
            acceptList.push_back(accept);
            SIPSDK_DETAILTRACE("ResipSipMessage::FillAccepts2SDK, Accept: " << accept);
        }
        this->SetAcceptList(acceptList);
    }
    
    return SIPSDK_ERROR_NOERROR;
}

CmResult ResipSipMessage::FillSessionExpires2SDK()
{
    SIPSDK_ASSERT_RETURN(this->mResipMessage, SIPSDK_ERROR_NULL_POINTER);
    
    if (this->mResipMessage->exists(h_SessionExpires))
    {
        UInt32 sessExpires = this->mResipMessage->header(h_SessionExpires).value();
        
        this->SetSessionExpires(sessExpires);
        SIPSDK_DETAILTRACE("ResipSipMessage::FillSessionExpires2SDK, Session-Expires: " << sessExpires);
    }
    
    if(this->mResipMessage->exists(h_MinSE))
    {
        UInt32 minSessExpires = this->mResipMessage->header(h_MinSE).value();
        
        this->SetMinSE(minSessExpires);
        SIPSDK_DETAILTRACE("ResipSipMessage::FillSessionExpires2SDK, Min-SE: " << minSessExpires );
    }

    return SIPSDK_ERROR_NOERROR;
}

CmResult ResipSipMessage::FillSupported2SDK()
{
    SIPSDK_ASSERT_RETURN(this->mResipMessage, SIPSDK_ERROR_NULL_POINTER);
    
    if (this->mResipMessage->exists(h_Supporteds))
    {
        StrContainer supportedList;
        for (Tokens::iterator iter = this->mResipMessage->header(h_Supporteds).begin();
                              iter != this->mResipMessage->header(h_Supporteds).end(); 
                              ++iter)
        {
            Token token = (*iter);
            supportedList.push_back(token.value().c_str());
            SIPSDK_DETAILTRACE("ResipSipMessage::FillSupported2SDK, Supported: " << token.value().c_str());
        }
        this->SetSupportedList(supportedList);
    }
    
    return SIPSDK_ERROR_NOERROR;
}

CmResult ResipSipMessage::FillCSeq2SDK()
{
    SIPSDK_ASSERT_RETURN(this->mResipMessage, SIPSDK_ERROR_NULL_POINTER);
    
    if (this->mResipMessage->exists(h_CSeq))
    {
        SipCSeqImpl * cseq = new SipCSeqImpl();
        cseq->SetMethod(::ConvertResipMethod2SDKMethod(this->mResipMessage->header(h_CSeq).method()));
        cseq->SetNumber(this->mResipMessage->header(h_CSeq).sequence());
        this->SetCSeq(cseq);
        
        SIPSDK_DETAILTRACE("ResipSipMessage::FillCSeq2SDK, CSeq: " << this->GetCSeq()->toString() << ", sequence: " << this->mResipMessage->header(h_CSeq).sequence());
    }
    
    return SIPSDK_ERROR_NOERROR;
}

CmResult ResipSipMessage::FillFrom2SDK()
{
    SIPSDK_ASSERT_RETURN(this->mResipMessage, SIPSDK_ERROR_NULL_POINTER);
    
    if (this->mResipMessage->exists(h_From))
    {
        SipFromImpl * from = new SipFromImpl();
        from->SetFrom(this->mResipMessage->header(h_From).uri().toString().c_str());
        
        if(this->mResipMessage->header(h_From).exists(p_tag))
            from->SetTag(this->mResipMessage->header(h_From).param(p_tag).c_str());
        
        this->SetFrom(from);
        SIPSDK_DETAILTRACE("ResipSipMessage::FillFrom2SDK, From: " << this->GetFrom()->toString());
    }
    
    return SIPSDK_ERROR_NOERROR;
}

CmResult ResipSipMessage::FillTo2SDK()
{
    SIPSDK_ASSERT_RETURN(this->mResipMessage, SIPSDK_ERROR_NULL_POINTER);
    
    if (this->mResipMessage->exists(h_To))
    {
        SipToImpl * to = new SipToImpl();
        to->SetTo(this->mResipMessage->header(h_To).uri().toString().c_str());
        if(this->mResipMessage->header(h_To).exists(p_tag))
            to->SetTag(this->mResipMessage->header(h_To).param(p_tag).c_str());
        this->SetTo(to);
     
        SIPSDK_DETAILTRACE("ResipSipMessage::FillTo2SDK, To: " << this->GetTo()->toString());
    }
    return SIPSDK_ERROR_NOERROR;
}

CmResult ResipSipMessage::FillRequsetLine2SDK()
{
    SIPSDK_ASSERT_RETURN(this->mResipMessage, SIPSDK_ERROR_NULL_POINTER);
    if(this->mResipMessage->isRequest())
    {
        this->SetIsSipRequest(true);
        
        this->SetRequestURI(this->mResipMessage->header(h_RequestLine).uri().toString().c_str());
        SIPSDK_DETAILTRACE("ResipSipMessage::FillRequsetLine2SDK, RequestURI: " << this->GetRequestURI());
        
        SipMethod method = ::ConvertResipMethod2SDKMethod(this->mResipMessage->header(h_RequestLine).getMethod());
        this->SetRequstMethod(method);
        SIPSDK_DETAILTRACE("ResipSipMessage::FillRequsetLine2SDK, RequestMethod: " << ConvertMethod2Str(this->GetRequstMethod()));
    }

    return SIPSDK_ERROR_NOERROR;
}

CmResult ResipSipMessage::FillResponseLine2SDK()
{
    SIPSDK_ASSERT_RETURN(this->mResipMessage, SIPSDK_ERROR_NULL_POINTER);
    if(this->mResipMessage->isResponse())
    {
        this->SetIsSipRequest(false);
        
        this->SetResponseStatusCode(this->mResipMessage->header(h_StatusLine).statusCode());
        SIPSDK_DETAILTRACE("ResipSipMessage::FillResponseLine2SDK, ResponseCode: " << this->GetResponseStatusCode());
        
        this->SetResponseReason(this->mResipMessage->header(h_StatusLine).reason().c_str());
        SIPSDK_DETAILTRACE("ResipSipMessage::FillResponseLine2SDK, ResponseReason: " << this->GetResponseReason());
    }

    return SIPSDK_ERROR_NOERROR;
}

CmResult ResipSipMessage::FillSdp2SDK()
{
    SIPSDK_INFOTRACE("ResipSipMessage::FillSdp2SDK, this: " << this << ", IsContainSdp: " << this->IsContainSdp());
    if(this->IsContainSdp())
    {
        SessionDescriptionImpl * sdkSDP = new SessionDescriptionImpl();
        this->SetSessionDescription(sdkSDP);

        Contents* body = this->mResipMessage->getContents();
        SIPSDK_ASSERT_RETURN(body, SIPSDK_ERROR_NULL_POINTER);
        
        // fill sdp raw content
        sdkSDP->SetRawContent(body->getBodyData().c_str());
        
        // fill sdp base info
        SdpContents* sdp = dynamic_cast<SdpContents*>(body);
        CmResult ret = this->FillSdpBase2SDK(sdp, sdkSDP);
        SIPSDK_ASSERT_RETURN(SIPSDK_ERROR_NOERROR == ret, ret);

        // fill sdp media info
        ISessionDescription::MediaDescrContainer& sdkMediaDescList = sdkSDP->GetMediaDescriptionList();
        ret = this->FillSdpMedia2SDK(sdp,  sdkMediaDescList);
        SIPSDK_ASSERT_RETURN(SIPSDK_ERROR_NOERROR == ret, ret);
    }

    return SIPSDK_ERROR_NOERROR;
}

CmResult ResipSipMessage::FillSdpBase2SDK(SdpContents* sdp, 
                                                 SessionDescriptionImpl * sdkSDP)
{
    SIPSDK_DETAILTRACE("ResipSipMessage::FillSdpBase2SDK, this: " << this << ", sdkSDP: " << sdkSDP);
    SIPSDK_ASSERT_RETURN(sdp,      SIPSDK_ERROR_NULL_POINTER);
    SIPSDK_ASSERT_RETURN(sdkSDP,   SIPSDK_ERROR_NULL_POINTER);
    
    // "i=" session info 
    sdkSDP->SetInfo(sdp->session().information().c_str());
    // "s=" session name
    sdkSDP->SetSessionName(sdp->session().name().c_str());
    // "u=" sesssion URI desc
    sdkSDP->SetURI(sdp->session().uri().toString().c_str());
    // "v=" protocol version
    sdkSDP->SetVersion(sdp->session().version());
    // "o=" session creator/indentifier
    this->FillSdpOrigin2SDK(sdp, sdkSDP);
    // "c=" session level connection
    this->FillSdpConn2SDK(sdp, sdkSDP);
    // "e=" session email list
    this->FillSdpEmail2SDK(sdp, sdkSDP);
    // "p=" session phone list
    this->FillSdpPhone2SDK(sdp, sdkSDP);
    // "t=" session time description
    this->FillSdpTime2SDK(sdp, sdkSDP);
    
    // session level attribute
    ISDAttribute* attr = sdkSDP->GetAttribute();
    SDAttributeImpl* sdkAttr = static_cast<SDAttributeImpl*>(attr);
    AttributeHelper& attrHelper = sdp->session().getAttribute();
    sdkAttr->SetAttributeHelp(attrHelper);

    return SIPSDK_ERROR_NOERROR;
}

CmResult ResipSipMessage::FillSdpOrigin2SDK(SdpContents* sdp, SessionDescriptionImpl * sdkSDP)
{
    SIPSDK_DETAILTRACE("ResipSipMessage::FillSdpOrigin2SDK, this: " << this << ", sdkSDP: " << sdkSDP);
    SIPSDK_ASSERT_RETURN(sdp,      SIPSDK_ERROR_NULL_POINTER);
    SIPSDK_ASSERT_RETURN(sdkSDP,   SIPSDK_ERROR_NULL_POINTER);
    
    SDOriginImpl* sdkOrigin = new SDOriginImpl();
    sdkSDP->SetOrigin(sdkOrigin);
    sdkOrigin->SetAddress(sdp->session().origin().getAddress().c_str());
    if(sdp->session().origin().getAddressType() == SdpContents::IP4)
        sdkOrigin->SetAddressType("IP4");
    else
        sdkOrigin->SetAddressType("IP6");
    sdkOrigin->SetNetType("IN");
    sdkOrigin->SetSessionID(sdp->session().origin().getSessionId());
    sdkOrigin->SetSessionVer(sdp->session().origin().getVersion());
    sdkOrigin->SetUserName(sdp->session().origin().user().c_str());

    return SIPSDK_ERROR_NOERROR;
}

CmResult ResipSipMessage::FillSdpConn2SDK(SdpContents* sdp, SessionDescriptionImpl * sdkSDP)
{
    SIPSDK_ASSERT_RETURN(sdp,      SIPSDK_ERROR_NULL_POINTER);
    SIPSDK_ASSERT_RETURN(sdkSDP,   SIPSDK_ERROR_NULL_POINTER);
    
    SDConnectionImpl* sdkConn = new SDConnectionImpl();
    sdkSDP->SetConnection(sdkConn);
    sdkConn->SetAddress(sdp->session().connection().getAddress().c_str());
    sdkConn->SetNetType("IN");
    if(sdp->session().connection().getAddressType() == SdpContents::IP4)
        sdkConn->SetAddressType("IP4");
    else
        sdkConn->SetAddressType("IP6");
    
    return SIPSDK_ERROR_NOERROR;
}

CmResult ResipSipMessage::FillSdpEmail2SDK(SdpContents* sdp, SessionDescriptionImpl * sdkSDP)
{
    SIPSDK_ASSERT_RETURN(sdp,      SIPSDK_ERROR_NULL_POINTER);
    SIPSDK_ASSERT_RETURN(sdkSDP,   SIPSDK_ERROR_NULL_POINTER);
    
    StrContainer& sdkEmailList = sdkSDP->GetEmailList();
    sdkEmailList.clear();
    const list<SdpContents::Session::Email>& emailList = sdp->session().getEmails();
    for(list<SdpContents::Session::Email>::const_iterator iter = emailList.begin(); 
                                                          iter != emailList.end(); 
                                                          iter ++)
    {
        SdpContents::Session::Email email = *iter;
        sdkEmailList.push_back(email.getAddress().c_str());
    }

    return SIPSDK_ERROR_NOERROR;
}

CmResult ResipSipMessage::FillSdpPhone2SDK(SdpContents* sdp, SessionDescriptionImpl * sdkSDP)
{
    SIPSDK_ASSERT_RETURN(sdp,      SIPSDK_ERROR_NULL_POINTER);
    SIPSDK_ASSERT_RETURN(sdkSDP,   SIPSDK_ERROR_NULL_POINTER);
    
    StrContainer& sdkPhoneList = sdkSDP->GetPhoneList();
    sdkPhoneList.clear();
    const list<SdpContents::Session::Phone>& phoneList = sdp->session().getPhones();
    for(list<SdpContents::Session::Phone>::const_iterator iter = phoneList.begin(); 
                                                          iter != phoneList.end(); 
                                                          iter ++)
    {
        SdpContents::Session::Phone phone = *iter;
        sdkPhoneList.push_back(phone.getNumber().c_str());
    }

    return SIPSDK_ERROR_NOERROR;
}

CmResult ResipSipMessage::FillSdpTime2SDK(SdpContents* sdp, SessionDescriptionImpl * sdkSDP)
{
    SIPSDK_ASSERT_RETURN(sdp,      SIPSDK_ERROR_NULL_POINTER);
    SIPSDK_ASSERT_RETURN(sdkSDP,   SIPSDK_ERROR_NULL_POINTER);
    
    // clear old data
    ISessionDescription::TimeDescrContainer& sdkTimeList = sdkSDP->GetTimeDescriptionList();
    for_each(sdkTimeList.begin(), sdkTimeList.end(), SIPSDKDeleteObj());
    ISessionDescription::TimeDescrContainer().swap(sdkTimeList);
    
    const list<SdpContents::Session::Time>& timeList = sdp->session().getTimes();
    for(list<SdpContents::Session::Time>::const_iterator iter = timeList.begin(); 
                                                         iter != timeList.end(); 
                                                         iter ++)
    {
        SdpContents::Session::Time time = *iter;
        ISDTimeDescription* timeDesc = ISDTimeDescription::Create();
        //timeDesc->AddRef();
        timeDesc->SetStartTime(time.getStart());
        timeDesc->SetStopTime(time.getStop());
        
        ISDTimeDescription::RTContainer& sdkRepeatList = timeDesc->GetRepeatTimeList();
        for_each(sdkRepeatList.begin(), sdkRepeatList.end(), SIPSDKDeleteObj());
        vector<ISDRepeatTime*>().swap(sdkRepeatList);

        const list<SdpContents::Session::Time::Repeat>& repeatList = time.getRepeats();
        for(list<SdpContents::Session::Time::Repeat>::const_iterator iter = repeatList.begin();
                                                                     iter != repeatList.end();
                                                                     iter ++)
        {
            SdpContents::Session::Time::Repeat repeat = *iter;
            SDRepeatTimeImpl* sdkRepeat = new SDRepeatTimeImpl;
            //sdkRepeat->AddRef();
            sdkRepeat->SetActiveDuration(repeat.getDuration());
            sdkRepeat->SetRepeatInterval(repeat.getInterval());
            vector<int> offsets;
            offsets.assign(repeat.getOffsets().begin(), repeat.getOffsets().end());
            sdkRepeat->SetOffset(offsets);
            sdkRepeatList.push_back(sdkRepeat);
        }

        sdkTimeList.push_back(timeDesc);
    }

    return SIPSDK_ERROR_NOERROR;
}

CmResult ResipSipMessage::FillSdpMedia2SDK(SdpContents* sdp, 
                                        ISessionDescription::MediaDescrContainer& sdkMediaDescList)
{
    SIPSDK_DETAILTRACE("ResipSipMessage::FillSdpMedia2SDK");
    SIPSDK_ASSERT_RETURN(sdp,  SIPSDK_ERROR_NULL_POINTER);

    // clear old
    for_each(sdkMediaDescList.begin(), sdkMediaDescList.end(), SIPSDKDeleteObj());
    vector<ISDMediaDescription*>().swap(sdkMediaDescList);

    // loop over the media(s) in resip
    const SdpContents::Session::MediumContainer& mediaList = sdp->session().media();
    for(SdpContents::Session::MediumContainer::const_iterator iter = mediaList.begin(); 
                                                              iter != mediaList.end(); 
                                                              iter ++)
    {
        SdpContents::Session::Medium media = *iter;
        SDMediaDescriptionImpl* sdkMediaDesc = new SDMediaDescriptionImpl;
        //sdkMediaDesc->AddRef();
        sdkMediaDescList.push_back(sdkMediaDesc);
        
        // "i=" media info
        sdkMediaDesc->SetInfo(media.information().c_str());

        // "c=" media level connection
        this->FillSdpMediaConn2SDK(sdkMediaDesc, media);

        // "a=" media level attribute
        ISDAttribute* attr = sdkMediaDesc->GetAttribute();
        SDAttributeImpl* sdkAttr = static_cast<SDAttributeImpl*>(attr);
        AttributeHelper& attrHelper = media.getAttribute();
        sdkAttr->SetAttributeHelp(attrHelper);

        // "m=" media name & transport
        this->FillSdpMediaMedium2SDK(sdkMediaDesc, media);

        // codec
        this->FillSdpMediaCodec2SDK(sdkMediaDesc, media);

        // crypto
        this->FillSdpMediaCrypto2SDK(sdkMediaDesc, media);
    }

    return SIPSDK_ERROR_NOERROR;
}

CmResult ResipSipMessage::FillSdpMediaConn2SDK(SDMediaDescriptionImpl* sdkMediaDesc,
                                                    const SdpContents::Session::Medium& media)
{
    SIPSDK_DETAILTRACE("ResipSipMessage::FillSdpMediaConn2SDK");
    SIPSDK_ASSERT_RETURN(sdkMediaDesc, SIPSDK_ERROR_NULL_POINTER);
    
    vector<ISDConnection*>& sdkMediaConnList = sdkMediaDesc->GetConnectionList();
    //clear old
    for_each(sdkMediaConnList.begin(), sdkMediaConnList.end(), SIPSDKDeleteObj());
    vector<ISDConnection*>().swap(sdkMediaConnList);
    
    const list<SdpContents::Session::Connection>& mediaConnList = media.getMediumConnections();
    for(list<SdpContents::Session::Connection>::const_iterator iter = mediaConnList.begin();
                                                               iter != mediaConnList.end();
                                                               iter ++)
    {
        SdpContents::Session::Connection conn = *iter;
        SDConnectionImpl* sdkMediaConn = new SDConnectionImpl;
        sdkMediaConn->AddRef();

        sdkMediaConn->SetAddress(conn.getAddress().c_str());
        if(conn.getAddressType()== SdpContents::IP4)
            sdkMediaConn->SetAddressType("IP4");
        else
            sdkMediaConn->SetAddressType("IP6");
        sdkMediaConn->SetNetType("IN");
        
        sdkMediaConnList.push_back(sdkMediaConn);
    }

    return SIPSDK_ERROR_NOERROR;
}

CmResult ResipSipMessage::FillSdpMediaMedium2SDK(SDMediaDescriptionImpl* sdkMediaDesc,
                                                    const SdpContents::Session::Medium& media)
{
    SIPSDK_DETAILTRACE("ResipSipMessage::FillSdpMediaMedium2SDK");
    SIPSDK_ASSERT_RETURN(sdkMediaDesc, SIPSDK_ERROR_NULL_POINTER);
    
    sdkMediaDesc->SetMediaPort(media.port());                       //media port
    sdkMediaDesc->SetMediaType(media.name().c_str());               //media type
    sdkMediaDesc->SetProtocol(media.protocol().c_str());            //media protocol
    
    vector<int>& sdkPayLoadList = sdkMediaDesc->GetPayLoadTypeList();  //media payload type format
    const list<Data>& formats = media.getFormats();
    for(list<Data>::const_iterator iter = formats.begin(); iter != formats.end(); iter ++)
        sdkPayLoadList.push_back(::atoi((*iter).c_str()));

    return SIPSDK_ERROR_NOERROR;
}

CmResult ResipSipMessage::FillSdpMediaCodec2SDK(SDMediaDescriptionImpl* sdkMediaDesc,
                                                    const SdpContents::Session::Medium& media)
{
    SIPSDK_DETAILTRACE("ResipSipMessage::FillSdpMediaCodec2SDK");
    SIPSDK_ASSERT_RETURN(sdkMediaDesc, SIPSDK_ERROR_NULL_POINTER);
    
    vector<ISDCodec*>& sdkCodecList = sdkMediaDesc->GetAllCodecs();   //media codecs
    const SdpContents::Session::Medium::CodecContainer& codecs = media.codecs();
    for(SdpContents::Session::Medium::CodecContainer::const_iterator iter = codecs.begin(); 
                                                                     iter != codecs.end(); iter ++)
    {
        SdpContents::Session::Codec codec = *iter;
        SDCodecImpl * sdkCodec = new SDCodecImpl();
        //sdkCodec->AddRef();
        sdkCodecList.push_back(sdkCodec);

        sdkCodec->SetName(codec.getName().c_str());
        sdkCodec->SetParameters(codec.parameters().c_str());
        sdkCodec->SetPayloadType(codec.payloadType());
        sdkCodec->SetRate(codec.getRate());
    }

    return SIPSDK_ERROR_NOERROR;
}

CmResult ResipSipMessage::FillSdpMediaCrypto2SDK(SDMediaDescriptionImpl* sdkMediaDesc,
                                                    const SdpContents::Session::Medium& media)
{
    
    SIPSDK_ASSERT_RETURN(sdkMediaDesc, SIPSDK_ERROR_NULL_POINTER);
    
    if(media.exists("crypto"))
    {
        
        const std::list<Data>& cryptos = media.getValues("crypto");
        SIPSDK_DETAILTRACE("@@@@@@@@@@@@@@@ResipSipMessage::FillSdpMediaCrypto2SDK: " << cryptos.size());

        cout << "@@@@@@@@@@@@@@@ResipSipMessage::FillSdpMediaCrypto2SDK: " << cryptos.size() << endl;
        std::list<Data>::const_iterator iter;
        for(iter = cryptos.begin(); iter != cryptos.end(); ++ iter)
        {
            ISDCrypto* crypto = ResipHelper::parseCryptoLine(*iter);
            if(crypto)
            {
                vector<ISDCrypto*>& cryptoList = sdkMediaDesc->GetAllCryptos();
                cryptoList.push_back(crypto);
            }
        }
    }

    return SIPSDK_ERROR_NOERROR;
}

SipMessage * ResipSipMessage::Sync2Resip()
{
    SipMessage * newResipMsg= NULL;
    CmResult ret = SIPSDK_ERROR_NOERROR;
    
    //if(this->mResipMessage == NULL)
    //    return this->Sync2NewResip();

    // creat new resip message
    if(this->IsSipRequest())
    {
        Uri toUri(this->GetTo()->toString().c_str());
        Uri fromUri(this->GetFrom()->toString().c_str());
        NameAddr target(toUri);
        NameAddr from(fromUri);
        NameAddr contact(fromUri);
        newResipMsg = Helper::makeRequest(target,from,contact,::ConvertSDKMethod2ResipMethod(this->GetRequstMethod()));
    }
    else
    {
        // require user create sip response based on request
        SIPSDK_ASSERT_RETURN(mResipMessage, NULL);
        newResipMsg = Helper::makeResponse(*mResipMessage, this->GetResponseStatusCode());
    }

    SIPSDK_DETAILTRACE("ResipSipMessage::Sync2Resip, create new resipMsg: " << newResipMsg<< ", Requset: " << this->IsSipRequest());
    
    // fill sip headers (SDK => resip)
    this->FillHeader2Resip(newResipMsg);

    ISessionDescription* localSDP = this->GetSessionDescription();
    if(this->IsContainSdp() && localSDP != NULL)
    {
        // fill sdp info into resip message from SDK
        //this->FillSdpBase2Resip();

        // fill sdp media into resip message from SDK
        //this->FillSdpMedia2Resip();

        string strSdp = localSDP->toString();

        HeaderFieldValue hfv(strSdp.c_str(), strSdp.size());
        Mime type("application", "sdp");
        SdpContents* sdp = new SdpContents(&hfv, type);
        auto_ptr<SdpContents> autoDel(sdp);
        try 
        {
            newResipMsg->setContents(sdp);
            //AttributeHelper attHelp = sdp->session().getAttribute();
            //attHelp = localSDP->GetAttribute();
        } 
        catch (ParseException& e)
        {
            SIPSDK_SAFE_DELETE(newResipMsg);
            SIPSDK_ERRTRACE("Parse sdp failed.");
            return NULL;
        }
    }
    else
    {
        string body = this->GetBodyContent();
        newResipMsg->setBody(body.c_str(), body.size());

        SIPSDK_DETAILTRACE("ResipSipMessage::Sync2Resip, msg body: \n" << body.c_str());
    }
    
    return newResipMsg;
}

CmResult ResipSipMessage::FillHeader2Resip(SipMessage* resipMsg)
{
    SIPSDK_ASSERT_RETURN(resipMsg, SIPSDK_ERROR_NULL_POINTER);
    SIPSDK_DETAILTRACE("ResipSipMessage::FillHeader2Resip, resipMsg: " << resipMsg);
    
    this->FillAlso2Resip(resipMsg);
    this->FillRefer2Resip(resipMsg);
    this->FillCallID2Resip(resipMsg);
    this->FillContact2Resip(resipMsg);
    this->FillContentType2Resip(resipMsg);
    this->FillVias2Resip(resipMsg);
    this->FillAllowEvents2Resip(resipMsg);
    this->FillRequire2Resip(resipMsg);
    this->FillEvent2Resip(resipMsg);
    this->FillAccepts2Resip(resipMsg);
    this->FillSessionExpires2Resip(resipMsg);
    this->FillSupported2Resip(resipMsg);
    this->FillCSeq2Resip(resipMsg);
    this->FillFrom2Resip(resipMsg);
    this->FillTo2Resip(resipMsg);
    this->FillRequsetLine2Resip(resipMsg);
    this->FillResponseLine2Resip(resipMsg);

    return SIPSDK_ERROR_NOERROR;
}

CmResult ResipSipMessage::FillAlso2Resip(SipMessage* resipMsg) 
{
    SIPSDK_ASSERT_RETURN(resipMsg, SIPSDK_ERROR_NULL_POINTER);

    string also = this->GetAlso();
    if (!also.empty())
    {
        resipMsg->header(h_Also).push_back(StringCategory(also.c_str()));
        SIPSDK_DETAILTRACE("ResipSipMessage::FillAlso2Resip, Also: " << also);
    }
    
    return SIPSDK_ERROR_NOERROR;
}

CmResult ResipSipMessage::FillRefer2Resip(SipMessage* resipMsg) 
{
    SIPSDK_ASSERT_RETURN(resipMsg, SIPSDK_ERROR_NULL_POINTER);

    string refer = this->GetRefer();
    if (!refer.empty())
    {
        resipMsg->header(h_Refer).push_back(StringCategory(refer.c_str()));
        SIPSDK_DETAILTRACE("ResipSipMessage::FillRefer2Resip, Refer: " << refer);
    }
    
    return SIPSDK_ERROR_NOERROR;
}

CmResult ResipSipMessage::FillCallID2Resip(SipMessage* resipMsg) 
{
    SIPSDK_ASSERT_RETURN(resipMsg, SIPSDK_ERROR_NULL_POINTER);

    string callID = this->GetCallID();
    if (!callID.empty())
    {
        resipMsg->header(h_CallId).value() = callID.c_str();
        SIPSDK_DETAILTRACE("ResipSipMessage::FillCallID2Resip, Call-ID: " << callID);
    }
    
    return SIPSDK_ERROR_NOERROR;
}

CmResult ResipSipMessage::FillContact2Resip(SipMessage* resipMsg) 
{
    SIPSDK_ASSERT_RETURN(resipMsg, SIPSDK_ERROR_NULL_POINTER);
    resipMsg->remove(h_Contacts); 

    ContactList contactList;
    this->GetContactList(contactList);
    
    if (!contactList.empty())
    {
        ContactIter iter = contactList.begin();
        for(; iter != contactList.end(); iter ++)
        {
            NameAddr contact;
            try
            {
                SipContactImpl* contact_impl = static_cast<SipContactImpl*>(*iter);
                if(!contact_impl)
                    continue;
                    
                contact_impl->toNameAddr(contact);
            }
            catch(ParseException & e)
            {
                SIPSDK_ERRTRACE("ResipSipMessage::FillContact2Resip, error: " << e.what());
                continue;
            }
                
            
            // Modified by Ellie, contact header MUST keep "tls" param when using TLS transport 
            if(SipProviderImpl::instance()->IsUsingTLS())
            	contact.uri().param(p_transport) = "tls";

            // Modified by Sam, contact header MUST keep "port" param
            // otherwise, the peer would not clear the port, and use the default 5060
            if(this->IsSipRequest())
                contact.uri().port() = SipProviderImpl::instance()->GetTransportPort();

            resipMsg->header(h_Contacts).push_back(contact);
            SIPSDK_DETAILTRACE("ResipSipMessage::FillContact2Resip, Contact: " << (*iter)->toString());
        }
    }

    return SIPSDK_ERROR_NOERROR;
}

CmResult ResipSipMessage::FillContentType2Resip(SipMessage* resipMsg) 
{
    SIPSDK_ASSERT_RETURN(resipMsg, SIPSDK_ERROR_NULL_POINTER);
    
    string contentType = this->GetContentType();
    size_t pos = contentType.find("/");
    if(pos != string::npos)
    {
        string type = contentType.substr(0, pos);
        string subType = contentType.substr(pos + 1);

        resipMsg->header(h_ContentType).type() = type.c_str(); 
        resipMsg->header(h_ContentType).subType() = subType.c_str();

        SIPSDK_DETAILTRACE("ResipSipMessage::FillContentType2Resip, contentType: " << contentType);
    }

    return SIPSDK_ERROR_NOERROR;
}

CmResult ResipSipMessage::FillVias2Resip(SipMessage* resipMsg) 
{
    SIPSDK_ASSERT_RETURN(resipMsg, SIPSDK_ERROR_NULL_POINTER);
    
    StrContainer viaList;
    this->GetViaList(viaList);
    if (viaList.size() > 0)
    {
        resipMsg->remove(h_Vias);
        for(StrContainerIter iter = viaList.begin(); iter != viaList.end(); iter ++)
        {
            string strVia = *iter;
            
            try
            {
                //SIPSDK_DETAILTRACE("ResipSipMessage::FillVias2Resip, strVia: " << strVia);
                Data data(strVia.c_str());
                ParseBuffer pb(data);
    
                Via via;
                via.parse(pb);
                
                via.param(p_branch);
                resipMsg->header(h_Vias).push_front(via);
            }
            catch(ParseException & e)
            {
                SIPSDK_ERRTRACE("ResipSipMessage::FillVias2Resip, error: " << e.what() << ", strVia: " << strVia);
                continue;
            }

            SIPSDK_DETAILTRACE("ResipSipMessage::FillVias2Resip, Via: " << strVia);
        }
    }

    return SIPSDK_ERROR_NOERROR;
}

CmResult ResipSipMessage::FillAllowEvents2Resip(SipMessage* resipMsg) 
{
    SIPSDK_ASSERT_RETURN(resipMsg, SIPSDK_ERROR_NULL_POINTER);
    
    StrContainer allowEventList;
    this->GetAllowEventList(allowEventList);
    if (allowEventList.size() > 0)
    {
        resipMsg->remove(h_AllowEvents);
        for(StrContainerIter iter = allowEventList.begin(); iter != allowEventList.end(); iter ++)
        {
            string strAllowEvt = *iter;

            if(strAllowEvt.empty())
                continue;
            
            Token token(strAllowEvt.c_str());
            resipMsg->header(h_AllowEvents).push_back(token);

            SIPSDK_DETAILTRACE("ResipSipMessage::FillAllowEvents2Resip, Allow-Events: " << strAllowEvt);
        }
    }

    return SIPSDK_ERROR_NOERROR;
}

CmResult ResipSipMessage::FillRequire2Resip(SipMessage* resipMsg) 
{
    SIPSDK_ASSERT_RETURN(resipMsg, SIPSDK_ERROR_NULL_POINTER);
    
    StrContainer requireList;
    this->GetRequireList(requireList);
    
    if (!requireList.empty())
    {
        resipMsg->remove(h_Requires);
        
        StrContainerIter iter = requireList.begin();
        for(; iter != requireList.end(); ++ iter)
        {
            string require = *iter;
            if(require.empty())
                continue;
            
            Token token(require.c_str());
            resipMsg->header(h_Requires).push_back(token);

            SIPSDK_DETAILTRACE("ResipSipMessage::FillRequire2Resip, Require: " << require);
        }
    }

    return SIPSDK_ERROR_NOERROR;
}

CmResult ResipSipMessage::FillEvent2Resip(SipMessage* resipMsg)
{
    SIPSDK_ASSERT_RETURN(resipMsg, SIPSDK_ERROR_NULL_POINTER);

    if (!this->GetEvent().empty())
        resipMsg->header(h_Event).value() = this->GetEvent().c_str();

    SIPSDK_DETAILTRACE("ResipSipMessage::FillEvent2Resip, Event: " << this->GetEvent().c_str());
    
    return SIPSDK_ERROR_NOERROR;
}
CmResult ResipSipMessage::FillAccepts2Resip(SipMessage* resipMsg)
{
    SIPSDK_ASSERT_RETURN(resipMsg, SIPSDK_ERROR_NULL_POINTER);
    
    StrContainer acceptList;
    this->GetAcceptList(acceptList);
    if (acceptList.size() > 0)
    {
        resipMsg->remove(h_Accepts);
        for(StrContainerIter iter = acceptList.begin(); iter != acceptList.end(); iter ++)
        {
            string accept = *iter;

            if(accept.empty())
                continue;

            size_t pos = accept.find("/");
            if(pos != string::npos)
            {
                string type = accept.substr(0, pos);
                string subType = accept.substr(pos + 1);

                Mime mime;
                mime.type() = type.c_str();
                mime.subType() = subType.c_str();
                
                resipMsg->header(h_Accepts).push_back(mime);
                SIPSDK_DETAILTRACE("ResipSipMessage::FillAccepts2Resip, Accept: " << accept);
            }
        }
    }

    return SIPSDK_ERROR_NOERROR;
}
CmResult ResipSipMessage::FillSessionExpires2Resip(SipMessage* resipMsg)
{
    SIPSDK_ASSERT_RETURN(resipMsg, SIPSDK_ERROR_NULL_POINTER);

    if (this->GetSessionExpires())
    {
        resipMsg->header(h_SessionExpires).value() = this->GetSessionExpires();
        switch(this->GetRefresher())
        {
            case SIPSDK_UAC:
                resipMsg->header(h_SessionExpires).param(p_refresher) = Data("uac");
                break;
            
            case SIPSDK_UAS:
                resipMsg->header(h_SessionExpires).param(p_refresher) = Data("uas");
                break;
        }
    }
    
    if (this->GetMinSE() != 0)
    {
        resipMsg->header(h_MinSE).value() = this->GetMinSE();
        SIPSDK_DETAILTRACE("ResipSipMessage::FillSessionExpires2Resip, Min-SE: " << this->GetMinSE());
    }
    
    return SIPSDK_ERROR_NOERROR;
}

CmResult ResipSipMessage::FillSupported2Resip(SipMessage* resipMsg) 
{
    SIPSDK_ASSERT_RETURN(resipMsg, SIPSDK_ERROR_NULL_POINTER);

    resipMsg->remove(h_Supporteds); 

    StrContainer supportedList;
    this->GetSupportedList(supportedList);
    if (supportedList.size() > 0)
    {
        for(StrContainerIter iter = supportedList.begin(); iter != supportedList.end(); iter ++)
        {
            string strSupported = *iter;

            if(strSupported.empty())
                continue;
            
            Token token(strSupported.c_str());
            resipMsg->header(h_Supporteds).push_back(token);

            SIPSDK_DETAILTRACE("ResipSipMessage::FillSupported2Resip, Supported: " << strSupported);
        }
    }

    return SIPSDK_ERROR_NOERROR;
}
CmResult ResipSipMessage::FillCSeq2Resip(SipMessage* resipMsg) 
{
    SIPSDK_ASSERT_RETURN(resipMsg, SIPSDK_ERROR_NULL_POINTER);
    
    if(this->GetCSeq()->GetNumber() > 0 && this->GetCSeq()->GetMethod() != SIP_METHOD_UNKNOWN)
    {
        resipMsg->header(h_CSeq).sequence() = this->GetCSeq()->GetNumber();

        string strMethod = ConvertMethod2Str(this->GetCSeq()->GetMethod());
        Data data(strMethod.c_str());
        resipMsg->header(h_CSeq).method() = getMethodType(data);

        SIPSDK_DETAILTRACE("ResipSipMessage::FillCSeq2Resip, CSeq: " << this->GetCSeq()->toString());
    }

    return SIPSDK_ERROR_NOERROR;
}

CmResult ResipSipMessage::FillFrom2Resip(SipMessage* resipMsg) 
{
    SIPSDK_ASSERT_RETURN(resipMsg, SIPSDK_ERROR_NULL_POINTER);

    if (!this->GetFrom()->toString().empty())
    {
        resipMsg->header(h_From) = NameAddr(this->GetFrom()->toString().c_str());

        //Modified by Ellie, tls is unnecessary
        //if(SipProviderImpl::instance()->IsUsingTLS())
        //    resipMsg->header(h_From).uri().param(p_transport) = "tls";
    }

    SIPSDK_DETAILTRACE("ResipSipMessage::FillFrom2Resip, From: " << this->GetFrom()->toString());
    
    return SIPSDK_ERROR_NOERROR;
}

CmResult ResipSipMessage::FillTo2Resip(SipMessage* resipMsg) 
{
    SIPSDK_ASSERT_RETURN(resipMsg, SIPSDK_ERROR_NULL_POINTER);

    if (!this->GetTo()->toString().empty())
    {
        resipMsg->header(h_To) = NameAddr(this->GetTo()->toString().c_str());
        
        //Modified by Ellie, tls is unnecessary
        //if(SipProviderImpl::instance()->IsUsingTLS())
        //    resipMsg->header(h_To).uri().param(p_transport) = "tls";
    }

    SIPSDK_DETAILTRACE("ResipSipMessage::FillTo2Resip, To: " << this->GetTo()->toString());
    
    return SIPSDK_ERROR_NOERROR;
}

CmResult ResipSipMessage::FillRequsetLine2Resip(SipMessage* resipMsg) 
{
    SIPSDK_ASSERT_RETURN(resipMsg, SIPSDK_ERROR_NULL_POINTER);
    
    if(this->IsSipRequest() && resipMsg->isRequest())
    {
        string requestURI = this->GetRequestURI();
        resipMsg->header(h_RequestLine).uri() = Uri(requestURI.c_str());

        string strMethod = ConvertMethod2Str(this->GetRequstMethod());
        Data data(strMethod.c_str());
        resipMsg->header(h_RequestLine).method() = getMethodType(data);

        SIPSDK_DETAILTRACE("ResipSipMessage::FillRequsetLine2Resip, requestURI: " << requestURI << ", strMethod: " << strMethod);
    }

    return SIPSDK_ERROR_NOERROR;
}

CmResult ResipSipMessage::FillResponseLine2Resip(SipMessage* resipMsg) 
{
    SIPSDK_ASSERT_RETURN(resipMsg, SIPSDK_ERROR_NULL_POINTER);
    
    if(!this->IsSipRequest() && !resipMsg->isRequest())
    {
        //SIPSDK_DETAILTRACE("ResipSipMessage::FillResponseLine2Resip " << this->GetResponseStatusCode() << ", " << resipMsg->isResponse() << " " << resipMsg->isRequest() );
        //this->mResipMessage->header(h_StatusLine).statusCode() = this->GetResponseStatusCode();
        //this->mResipMessage->header(h_StatusLine).reason() = this->GetResponseReason().c_str();
        //SIPSDK_DETAILTRACE("ResipSipMessage::FillResponseLine2Resip " << this->GetResponseStatusCode() << ", " << resipMsg->isResponse() << " " << resipMsg->isRequest() );
    }

    return SIPSDK_ERROR_NOERROR;
}

string ResipSipMessage::EncodeResipMsg(SipMessage * msg) 
{
    if(!msg)
        return "";
    
    Data encoded;
    DataStream encodeStream(encoded);
    msg->encode(encodeStream);
    encodeStream.flush();

    return encoded.c_str();
}

/////////////////////////////////////////////////////////////////////
////SipStackAdapter
SipStackAdapter * SipStackAdapter::CreateResipStackAdapter()
{
    return new ResipStackAdapterImpl();
}


/////////////////////////////////////////////////////////////////////
////ResipStatsHandler
ResipStatsHandler::ResipStatsHandler() : mResipStat(NULL)
{
    SIPSDK_CLASSTRACE("ResipStatsHandler::ResipStatsHandler");
}

ResipStatsHandler::~ResipStatsHandler()
{
    SIPSDK_CLASSTRACE("ResipStatsHandler::~ResipStatsHandler");

    if(mResipStat)
    {
        mResipStat->ReleaseRef();
        mResipStat = NULL;
    }
}

void ResipStatsHandler::SetResipStat(ResipStat * stat)
{
    if(mResipStat)
    {
        mResipStat->ReleaseRef();
        mResipStat = NULL;
    }

    mResipStat = stat;
    mResipStat->AddRef();
}

bool ResipStatsHandler::operator()(StatisticsMessage& statsMessage)
{
    // Ricky fixed the bug, 2011-11-22 10:16:53
    if(!mResipStat)
    {
        SIPSDK_ERRTRACE("mResipStat is NULL.");
        return false;
    }
        
    StatisticsMessage::Payload& payLoad = mResipStat->GetPayLoad();
    statsMessage.loadOut(payLoad);
        
    return true;
}


/////////////////////////////////////////////////////////////////////
////ResipStat
ResipStat::ResipStat() : mPayLoad()
{
    SIPSDK_CLASSTRACE("ResipStat::ResipStat");
}

ResipStat::~ResipStat()
{
    SIPSDK_CLASSTRACE("ResipStat::~ResipStat");
}

void ResipStat::AddRef()
{
    CReferenceControl::add_reference();
}

void ResipStat::ReleaseRef()
{
    CReferenceControl::release_reference();
}

StatisticsMessage::Payload& ResipStat::GetPayLoad()
{
    return this->mPayLoad;
}

uint32 ResipStat::GetValue(StackStat::StatType type) const
{
    switch(type)
    {
        case TU_FIFO_SIZE:
            return mPayLoad.tuFifoSize;
            break;
        case TRANSPORT_FIFO_SIZE_SUM:
            return mPayLoad.transportFifoSizeSum;
            break;
        case TRANSACTION_FIFO_SIZE:
            return mPayLoad.transactionFifoSize;
            break;
        case ACTIVE_TIMER_NUM:
            return mPayLoad.activeTimers;
            break;
        case OPEN_TCP_CONNECTION_NUM:
            return mPayLoad.openTcpConnections;
            break;
        case ACTIVE_CLIENT_TRANSACTION_NUM:
            return mPayLoad.activeClientTransactions;
            break;
        case ACTIVE_SERVER_TRANSACTION_NUM:
            return mPayLoad.activeServerTransactions;
            break;
        case PENDING_DNS_QUERY_NUM:
            return mPayLoad.pendingDnsQueries;
            break;
        case SENT_REQUEST_NUM:
            return mPayLoad.requestsSent;
            break;
        case SENT_RESPONSE_NUM:
            return mPayLoad.responsesSent;
            break;
        case RETRANSMIT_REQUEST_NUM:
            return mPayLoad.requestsRetransmitted;
            break;
        case RETRANSMIT_RESPONSE_NUM:
            return mPayLoad.responsesRetransmitted;
            break;
        case RECEIVE_REQUEST_NUM:
            return mPayLoad.requestsReceived;
            break;
        case RECEIVER_RESPONSE_NUM:
            return mPayLoad.responsesReceived;
            break;
        default:
            SIPSDK_ERRTRACE("Unsupported stat type: " << type);
            break;
    }
        
    return 0;
}

uint32 ResipStat::Get2xxInSum(SipMethod method) const
{
    return mPayLoad.sum2xxIn(::ConvertSDKMethod2ResipMethod(method));
}

uint32 ResipStat::GetErrInSum(SipMethod method) const
{
    return mPayLoad.sumErrIn(::ConvertSDKMethod2ResipMethod(method));
}

uint32 ResipStat::Get2xxOutSum(SipMethod method) const
{
    return mPayLoad.sum2xxOut(::ConvertSDKMethod2ResipMethod(method));
}

uint32 ResipStat::GetErrOutSum(SipMethod method) const
{
    return mPayLoad.sumErrOut(::ConvertSDKMethod2ResipMethod(method));
}

/////////////////////////////////////////////////////////////////////
////ResipStackAdapterImpl
ResipStackAdapterImpl::ResipStackAdapterImpl() :
    mRunFlag(true),
    mStack(NULL),
    mFdSet(),
    mHasInit(false),
    mHasAddTransport(false),
    mStackStatHandler(NULL),
    mStackStat(NULL),
    mCertPath(),
    mTlsDomain(),
    mIsUsingTLS(false),
    mTransportPort(0),
    mTransportName("UDP"),
    mPollGrp(NULL)
{
    SIPSDK_CLASSTRACE("SipProviderImpl::ResipStackAdapterImpl");

    mStackStatHandler = new ResipStatsHandler;

    mStackStat = new ResipStat;
    mStackStat->AddRef();

    mStackStatHandler->SetResipStat(mStackStat);
}

ResipStackAdapterImpl::~ResipStackAdapterImpl()
{
    SIPSDK_CLASSTRACE("SipProviderImpl::~ResipStackAdapterImpl");

    SIPSDK_SAFE_DELETE(mStackStatHandler);

    if(mStackStat)
    {
        mStackStat->ReleaseRef();
        mStackStat = NULL;
    }

    if(mPollGrp)
    {
        //delete mPollGrp;
        mPollGrp = NULL;
    }

    SIPSDK_SAFE_DELETE(mStack);
}

void ResipStackAdapterImpl::AddRef()
{
    CReferenceControl::add_reference();
}

void ResipStackAdapterImpl::ReleaseRef()
{
    CReferenceControl::release_reference();
}

int ResipStackAdapterImpl::SetOpt(SipOption opt, void * s, size_t n)
{
    int res = -1;
    
    switch(opt)
    {
    case QOS_VALUE:
        res = this->SetQoS(s, n);
        break;
    
    default:
        break;
    }
    
    return res;
}

int ResipStackAdapterImpl::SetQoS(void * s, size_t n)
{
    if(!s || n != sizeof(int))
        return -1;
    
    int dscpValue = *(int *)s;
    ResipHelper::gDSCPValue = dscpValue;

    // TODO: to change all existing SIP connections socket option value here
    // 2011-10-11 11:54:02
    return 0;
}

CmResult ResipStackAdapterImpl::Init()
{
    SIPSDK_INFOTRACE("ResipStackAdapterImpl::Init");
    if(mHasInit)
        return SIPSDK_ERROR_NOERROR;

    if(mStack == NULL)
    {
        SIPSDK_ERRTRACE("Resip stack adapter init failed, please add transport first.");
        return SIPSDK_ERROR_NOERROR;
    }
    
    try
    {
        //mStack.setStatisticsInterval(5);
        mStack->setExternalStatsHandler(mStackStatHandler);
        //mStack.buildFdSet(mFdSet);
        //INFOTRACE("mStack.getTimeTillNextProcessMS: " << mStack.getTimeTillNextProcessMS());
        //mFdSet.selectMilliSeconds( 1000 );
        //mStack.process(mFdSet);
        
        mHasInit = true;
    }
    catch(BaseException& e)
    {
        SIPSDK_ERRTRACE("ResipStackAdapterImpl::Init exception, name= "<<e.name()
            <<" message is "<<e.getMessage().c_str());
        return SIPSDK_ERROR_IO_FAILURE;
    }
    return SIPSDK_ERROR_NOERROR;
}

CmResult ResipStackAdapterImpl::InitLog(CStrRef path, CStrRef filePrefix, StackLogLevel level)
{
    if(path.empty())
    {
        SIPSDK_ERRTRACE("ResipStackAdapterImpl::InitLog, path is empty.");
        return SIPSDK_ERROR_NOERROR;
    }
    
    string logFile = path;
    string prefix = filePrefix;

    if(prefix.empty())
        prefix = "resiprocate";
    
    if(logFile.at(logFile.length() - 1) != '/')
        logFile.append("/");
    logFile.append(prefix);

    char szDate[16] = {0};
    time_t now = ::time(NULL);
    ::strftime(szDate, sizeof(szDate), "_%m%d%Y_", ::localtime(&now));

    logFile.append(szDate);
    
    char szPid[16] = {0};
    ::sprintf(szPid, "%d", ::getpid());
    
    logFile.append(szPid);
    logFile.append(".log");

    Log::initialize(Log::File, ConvertLogLevel(level), "sipsdk", logFile.c_str());
    
    return SIPSDK_ERROR_NOERROR;
}

resip::Log::Level ResipStackAdapterImpl::ConvertLogLevel(StackLogLevel level)
{
    resip::Log::Level logLevel = resip::Log::Info;
    switch (level)
    {
        case STACK_LOG_NONE:
            logLevel = resip::Log::None;
            break;
        case STACK_LOG_ERROR:
            logLevel = resip::Log::Warning;
            break;
        case STACK_LOG_INFO:
            logLevel = resip::Log::Info;
            break;
        case STACK_LOG_DETAIL:
            logLevel = resip::Log::Stack;
            break;
    }

    return logLevel;
}

/*
CmResult ResipStackAdapterImpl::Run()
{
    return 0;
}
*/
CmResult ResipStackAdapterImpl::Shutdown()
{
    this->mRunFlag = false;
    mStack->shutdown();
    return SIPSDK_ERROR_NOERROR;
}
/*
CmResult ResipStackAdapterImpl::Join()
{
    return 0;
}
*/
CmResult ResipStackAdapterImpl::SendMsg(SipMessage * resipMsg)
{
    SIPSDK_ASSERT_RETURN(resipMsg, SIPSDK_ERROR_NULL_POINTER);

    try
    {
        SIPSDK_INFOTRACE("ResipStackAdapterImpl::SendMsg:\n\n" << ResipSipMessage::EncodeResipMsg(resipMsg));
        mStack->send(*resipMsg);
    } 
    catch (exception& e)
    {
        SIPSDK_ERRTRACE("ResipStackAdapterImpl::SendMsg, what(): " << e.what());
        return SIPSDK_ERROR_SEND_MSG_EXCEPTION;
    }
  
    return SIPSDK_ERROR_NOERROR;
}

ISipMessage * ResipStackAdapterImpl::RecvMsg()
{
    // Added by Sam, adding mRunFlag to exit loop after shutdown stack
    try
    {
        mStack->buildFdSet(mFdSet);
        mFdSet.selectMilliSeconds(resipMin(mStack->getTimeTillNextProcessMS(), (unsigned int)50) );
        mStack->process(mFdSet);

        // receive SipMessage from stack, would release it after process completely
        SipMessage* resipMsg = NULL;
        while (mRunFlag && (resipMsg = mStack->receive()) != NULL)
        {
            SIPSDK_INFOTRACE("ResipStackAdapterImpl::RecvMsg, this: " << this << "\n\n" << ResipSipMessage::EncodeResipMsg(resipMsg));
            ResipSipMessage* sdkMsg = new ResipSipMessage(resipMsg);

            // convert the resip sip message into SDK sip message
            if (SIPSDK_ERROR_NOERROR != sdkMsg->Sync2SDK())
            {
                SIPSDK_ERRTRACE("ResipSipMessage::Sync2SDK failed, discard message: " << this);

                SIPSDK_SAFE_DELETE(resipMsg);
                return NULL;
            }
            
            return sdkMsg;
        }
    }
    catch (exception& e)
    {
        SIPSDK_ERRTRACE("ResipStackAdapterImpl::RecvMsg, what(): " << e.what());
    }

    return NULL;
}

void ResipStackAdapterImpl::SetTlsCertPath(string path)
{
    this->mCertPath = path;
}
void ResipStackAdapterImpl::SetTlsDomain(string domain)
{
    this->mTlsDomain = domain;
}

CmResult ResipStackAdapterImpl::AddTransport(TransportProtocol protocol,
                                            CStrRef ipAddress,
                                            int port,
                                            AddressType type,
                                            QosType dscpValue)
{
    if(mHasAddTransport)
    {
        SIPSDK_ERRTRACE("Add transport already!");
        return SIPSDK_ERROR_ADD_TRANSPORT_ERROR;
    }
    
    TransportType transportType = (TransportType)protocol;
    IpVersion ipVer = (IpVersion)type;

    Data bindAddress = Data::Empty;
    if(!ipAddress.empty())
        bindAddress = ipAddress.c_str();

    try
    {
        if(protocol == SIP_TRANSPORT_TLS)
        {
            if(mCertPath.empty() || mTlsDomain.empty())
            {
                SIPSDK_ERRTRACE("CertPath and TLS domain should not be empty for TLS transport, mCertPath: " << mCertPath << ", mTlsDomain: " << mTlsDomain);
                return SIPSDK_ERROR_ADD_TRANSPORT_ERROR;
            }

            mPollGrp = FdPollGrp::create("epoll");
            EventThreadInterruptor* mEventIntr = new EventThreadInterruptor(*mPollGrp);
            
            SipStackOptions options;
            options.mAsyncProcessHandler = mEventIntr;
            options.mPollGrp = mPollGrp;
            options.mSecurity = new Security(mCertPath.c_str());
            
            // Ricky, 2011-10-20 14:35:38 for DSCP
            options.mSocketFunc = &ResipHelper::afterSocketCreation;
            ResipHelper::gDSCPValue = dscpValue;

            /*
            // Add DNS -- debug
            options.mExtraNameserverList = new DnsStub::NameserverList();
            DnsStub::NameserverList* tmp = (DnsStub::NameserverList*)options.mExtraNameserverList;

            Tuple tuple("10.224.30.4:53", 0, V4);
            tmp->push_back(tuple.toGenericIPAddress());
            // End add
            */
            
            mStack = new SipStack(options);
            
            //Security* security = new Security(mCertPath.c_str());
            //mStack = new SipStack(security);

            mStack->addTransport(TLS, port, ipVer, StunDisabled, bindAddress, mTlsDomain.c_str());
            mIsUsingTLS = true;
            mTransportName = "TLS";
        }
        else
        {
            mPollGrp = FdPollGrp::create("epoll");
            EventThreadInterruptor* mEventIntr = new EventThreadInterruptor(*mPollGrp);
            SipStackOptions options;
            options.mAsyncProcessHandler = mEventIntr;
            options.mPollGrp = mPollGrp;

            // Ricky, 2011-10-20 14:35:38 for DSCP
            options.mSocketFunc = &ResipHelper::afterSocketCreation;
            ResipHelper::gDSCPValue = dscpValue;

            mStack = new SipStack(options);
            mStack->addTransport(transportType, port, ipVer, StunDisabled, bindAddress);

            mTransportName = (protocol == SIP_TRANSPORT_TCP ? "TCP" : "UDP");
        }

        mTransportPort = port;
    }
    catch (BaseException& e)
    {
        SIPSDK_ERRTRACE("ResipStackAdapterImpl::AddTransport exception, name= "<<e.name()
            <<" message is "<<e.getMessage().c_str());
        return SIPSDK_ERROR_ADD_TRANSPORT_ERROR;
    }

    mHasAddTransport = true;
    return SIPSDK_ERROR_NOERROR;
}

bool ResipStackAdapterImpl::IsUsingTLS() const
{
    return mIsUsingTLS;
}

StackStat* ResipStackAdapterImpl::GetStackStat()
{
    return this->mStackStat;
}

int ResipStackAdapterImpl::GetTransportPort()
{
    return this->mTransportPort;
}

CStrRef ResipStackAdapterImpl::GetTransportName()
{   
    return this->mTransportName;
}

QosType ResipHelper::gDSCPValue = 0;

ISDCrypto* ResipHelper::parseCryptoLine(const Data& cryptoLine)
{
    SIPSDK_DETAILTRACE("ResipHelper::parseCryptoLine: " << cryptoLine.c_str());  
    ISDCrypto* crypto = NULL;

    ParseBuffer pb(cryptoLine);
    unsigned int tag = pb.uInt32();
    Data token;

    pb.skipToChar(Symbols::SPACE[0]);
    const char * anchor = pb.skipWhitespace();
    pb.skipToChar(Symbols::SPACE[0]);
    pb.data(token, anchor);

    string suite = token.c_str();
    SIPSDK_DETAILTRACE("ResipHelper::parseCryptoLine, suite: " << suite);

    if( suite != "AES_CM_128_HMAC_SHA1_80" && 
        suite != "AES_CM_128_HMAC_SHA1_32" && 
        suite != "F8_128_HMAC_SHA1_80")
        SIPSDK_ERRTRACE("invalid crypto line: " << cryptoLine.c_str());
    

    crypto = ISDCrypto::Create();
    crypto->SetTag(tag);
    crypto->SetCryptoSuiteType(suite);
    vector<ISDCryptoParam*>& keyParamList = crypto->GetAllParams();

    anchor = pb.skipWhitespace();

    // Loop through KeyParams
    while(true)
    {
        string method;
        Data base64Key;
        unsigned int lifetime = 0;
        unsigned int mkiValue = 0;
        unsigned int mkiLength = 0;

        parseCryptoParams(pb, method, base64Key, lifetime, mkiValue, mkiLength);

        ISDCryptoParam* param = ISDCryptoParam::Create();

        string rawKey = Base64Coder::decode(base64Key).c_str();
        string masterKey = rawKey.substr(0, 16);
        string masterSalt = rawKey.substr(16);

        SIPSDK_DETAILTRACE("ResipHelper::parseCryptoLine, base64Key: " << base64Key.c_str() 
            << ", rawKey: " << rawKey << ", masterKey: " << masterKey << ", masterSalt: " << masterSalt);
        
        param->SetMasterKey(masterKey);
        param->SetMasterSalt(masterSalt);
        param->SetLifeTime(lifetime);
        param->SetMasterKeyIndex(mkiValue);
        param->SetMasterKeyIndexLen(mkiLength);
        keyParamList.push_back(param);

        if(pb.eof() || *pb.position() != ';') break;
        anchor = pb.skipChar();
    }
    /*
    while(!pb.eof())
    {
        anchor = pb.skipWhitespace();
        pb.skipToOneOf(" =");
        pb.data(token, anchor);
        if(token == "KDR")
        {
            pb.skipChar();
            crypto->setSrtpKdr(pb.uInt32());
        }
        else if(token == "UNENCRYPTED_SRTP")
        {
            crypto->setEncryptedSrtp(false);
        }
        else if(token == "UNENCRYPTED_SRTCP")
        {
            crypto->setEncryptedSrtcp(false);
        }
        else if(token == "UNAUTHENTICATED_SRTP")
        {
            crypto->setAuthenticatedSrtp(false);
        }
        else if(token == "FEC_ORDER")
        {
            anchor = pb.skipChar();
            pb.skipToChar(Symbols::SPACE[0]);
            pb.data(token, anchor);
            crypto->setSrtpFecOrder(SdpMediaLine::SdpCrypto::getSrtpFecOrderFromString(token.c_str()));
        }
        else if(token == "FEC_KEY")
        {
            anchor = pb.skipChar();
            SdpMediaLine::SdpCryptoKeyMethod method;
            Data keyValue;
            unsigned int lifetime = 0;
            unsigned int mkiValue = 0;
            unsigned int mkiLength = 0;

            parseCryptoParams(pb, method, keyValue, lifetime, mkiValue, mkiLength);
            crypto->setSrtpFecKey(method, keyValue.c_str(), lifetime, mkiValue, mkiLength);
        }
        else if(token == "WSH")
        {
            pb.skipChar();
            crypto->setSrtpWsh(pb.uInt32());
        }
        else
        {
            if(!pb.eof() && *pb.position() == '=')
            {
                pb.skipToChar(Symbols::SPACE[0]);
                pb.data(token, anchor);
            }
            crypto->addGenericSessionParam(token.c_str());
        }
    }
    */

    return crypto;
}

void ResipHelper::afterSocketCreation(Socket s, int transportType, const char* file, int line)
{
    int res = 0;
    
#ifndef WIN32
    res = ::setsockopt(s, SOL_IP, IP_TOS, &ResipHelper::gDSCPValue, sizeof(ResipHelper::gDSCPValue));
#endif

}

void ResipHelper::parseCryptoParams(ParseBuffer& pb, 
                                       string& keyMethod, 
                                       Data& keyValue, 
                                       unsigned int& srtpLifetime, 
                                       unsigned int& srtpMkiValue, 
                                       unsigned int& srtpMkiLength)
{
    //inline:d0RmdmcmVCspeEc3QGZiNWpVLFJhQX1cfHAwJSoj|2^20|1:32
    const char * anchor = pb.position();
    Data token;
    pb.skipToChar(Symbols::COLON[0]);
    pb.data(token, anchor);
    keyMethod = token.c_str();
    if(keyMethod == "inline")
    {
        anchor = pb.skipChar();
        pb.skipToOneOf("|;", Symbols::SPACE);
        pb.data(keyValue, anchor);
        
        if(!pb.eof() && *pb.position() == '|')
        {
            pb.skipChar();
            unsigned int base = pb.uInt32();
            pb.skipToOneOf("^|:;", Symbols::SPACE);
            
            if(*pb.position() == '^')
            {
                pb.skipChar();
                unsigned int exponent = pb.uInt32();  
                srtpLifetime = base;
                
                while(0 != --exponent) 
                    srtpLifetime *= base;
                
                pb.skipToOneOf("|;", Symbols::SPACE);
                
                if(!pb.eof() && *pb.position() == '|')
                {
                    pb.skipChar();
                    base = pb.uInt32();
                    pb.skipToOneOf(":;", Symbols::SPACE);
                }
            }
            else if(*pb.position() != ':')
            {
                srtpLifetime = base;
                
                if(!pb.eof())
                {
                    if(*pb.position() == '|')
                    {
                        pb.skipChar();
                        base = pb.uInt32();
                    }
                    
                    pb.skipToOneOf(":;", Symbols::SPACE);
                }
            }
            if(!pb.eof() && *pb.position() == ':')
            {
                srtpMkiValue = base;
                pb.skipChar();
                srtpMkiLength = pb.uInt32();
                pb.skipToOneOf(";", Symbols::SPACE);
            }
        }
    }
    else
    {
        anchor = pb.skipChar();
        pb.skipToChar(Symbols::SPACE[0]);
        pb.data(keyValue, anchor);

        if(!pb.eof())
        {
            pb.skipToOneOf(";", Symbols::SPACE);
        }
    }
}

SipMethod ConvertResipMethod2SDKMethod(MethodTypes method) 
{
    Data methodName = ::getMethodName(method);
    return ::ConvertStr2Method(methodName.c_str());
}

MethodTypes ConvertSDKMethod2ResipMethod(SipMethod method) 
{
    string strMethod = ::ConvertMethod2Str(method);
    Data data(strMethod.c_str());
    return ::getMethodType(data);
}

