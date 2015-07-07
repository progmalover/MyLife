#include <gtest/gtest.h>
//#include "gtest_defines.h"
#include "ResipStackAdapterImpl.h"
#include "resip/stack/SipMessage.hxx"
using namespace resip;

class TestResipSipMessage : public ::testing::Test
{
public:
    TestResipSipMessage() {}
    ~TestResipSipMessage() {}
    
    virtual void SetUp()
    {
    }
 
    virtual void TearDown()
    {
    }

};

#define _CRLF "\r\n"

TEST_F (TestResipSipMessage, Sync2SDK_Request)
{
    string msg_content = 
    "INVITE sip:sjphone@10.224.55.26:5060 SIP/2.0"_CRLF
    "Via: SIP/2.0/UDP 10.224.70.94:49152;branch=z9hG4bK0ae0465e000000244e1d1435000027da00000007;rport"_CRLF
    "From: \"unknown\" <sip:6436@10.224.54.213>;tag=2f56ab7148"_CRLF
    "To: <sip:sjphone@10.224.55.26:5060>"_CRLF
    "Contact: <sip:6436@10.224.70.94:49152>"_CRLF
    "Call-ID: 4149A002F86140CE9FF07EA4CC39D0170x0ae0465e"_CRLF
    "CSeq: 1 INVITE"_CRLF
    "Max-Forwards: 70"_CRLF
    "User-Agent: SJphone/1.65.377a (SJ Labs)"_CRLF
    "Content-Length: 365"_CRLF
    "Content-Type: application/sdp"_CRLF
    "Supported: replaces,norefersub,timer"_CRLF
    _CRLF;
    string sdp_content = 
    "v=0"_CRLF
    "o=- 3519517365 3519517365 IN IP4 10.224.70.94"_CRLF
    "s=SJphone"_CRLF
    "c=IN IP4 10.224.70.94"_CRLF
    "t=0 0"_CRLF
    "m=audio 49154 RTP/AVP 3 97 98 8 0 101"_CRLF
    "c=IN IP4 10.224.70.94"_CRLF
    "a=rtpmap:3 GSM/8000"_CRLF
    "a=rtpmap:97 iLBC/8000"_CRLF
    "a=rtpmap:98 iLBC/8000"_CRLF
    "a=fmtp:98 mode=20"_CRLF
    "a=rtpmap:8 PCMA/8000"_CRLF
    "a=rtpmap:0 PCMU/8000"_CRLF
    "a=rtpmap:101 telephone-event/8000"_CRLF
    "a=fmtp:101 0-16"_CRLF
    "a=setup:active"_CRLF
    "a=sendrecv"_CRLF;
    msg_content += sdp_content;
    cout << msg_content << endl;
    SipMessage *msg = SipMessage::make(Data(msg_content.c_str()));
    cout << (void *)msg << endl;
    ResipSipMessage resipMsg(msg);
    EXPECT_TRUE(resipMsg.Sync2SDK());
    EXPECT_EQ(resipMsg.GetRequstMethod(), SIP_METHOD_INVITE);
    EXPECT_EQ(resipMsg.GetRequestURI(), "sip:sjphone@10.224.55.26:5060");
    EXPECT_EQ(resipMsg.GetCallID(), "4149A002F86140CE9FF07EA4CC39D0170x0ae0465e");
    ISipCSeq *cseq = resipMsg.GetCSeq();
    EXPECT_EQ(cseq->GetMethod(), SIP_METHOD_INVITE);
    EXPECT_EQ(cseq->GetNumber(), 1);
    ISipFrom *from = resipMsg.GetFrom();
    EXPECT_EQ(from->GetFrom(), "sip:6436@10.224.54.213");
    EXPECT_EQ(from->GetTag(), "2f56ab7148");
    ISipTo *to = resipMsg.GetTo();
    EXPECT_EQ(to->GetTo(), "sip:sjphone@10.224.55.26:5060");
    EXPECT_EQ(to->GetTag(), "");
    StrContainer contacts;
    resipMsg.GetContactList(contacts);
    EXPECT_EQ(contacts.size(), 1);
    EXPECT_EQ(contacts[0], "sip:6436@10.224.70.94:49152");
    StrContainer vias;
    resipMsg.GetViaList(vias);
    EXPECT_EQ(vias.size(), 1);
    EXPECT_EQ(vias[0], "SIP/2.0/UDP 10.224.70.94:49152;branch=z9hG4bK0ae0465e000000244e1d1435000027da00000007;rport");
    EXPECT_EQ(resipMsg.GetContentType(), "application/sdp");
/*    ISessionDescription *sdp = resipMsg.GetSessionDescription();
    EXPECT_EQ(sdp->GetRawContent(), sdp_content);
    EXPECT_EQ(sdp->GetVersion(), 0);
    EXPECT_EQ(sdp->GetSessionName(), "SJphone");
    EXPECT_EQ(sdp->GetInfo(), "");
    EXPECT_EQ(sdp->GetURI(), "");*/
    /*EXPECT_EQ(sdp->GetOrigin(), 0);
    EXPECT_EQ(sdp->GetConnection(), 0);
    EXPECT_EQ(sdp->GetEmailList(), 0);
    EXPECT_EQ(sdp->GetPhoneList(), 0);
    EXPECT_EQ(sdp->GetTimeDescriptionList(), 0);
    EXPECT_EQ(sdp->GetMediaDescriptionList(), 0);
    EXPECT_EQ(sdp->GetAttribute(), 0);*/
}

TEST_F (TestResipSipMessage, Sync2Resip_Request)
{
    /*
    ResipSipMessage request;
    request.SetRequstMethod(SIP_METHOD_INVITE);
    request.SetRequestURI("sip:sjphone@10.224.55.26:5060");
    request.SetCallID("BF287C4CE9E8468B95AADEDFEAD55BF70x0ae0465e");
    request.SetCSeq(new SipCSeqImpl("1 INVITE"));
    request.SetFrom(new SipFromImpl("\"unknown\" <sip:6436@10.224.54.213>;tag=78252a99ab9f"));
    request.SetTo(new SipToImpl("<sip:sjphone@10.224.55.26:5060>"));
    StrContainer contacts;
    contacts.push_back("contact1");
    contacts.push_back("contact2");
    contacts.push_back("contact3");
    request.SetContactList(contacts);
    StrContainer vias;
    vias.push_back("via1");
    vias.push_back("via2");
    vias.push_back("SIP/2.0/UDP 10.224.70.94:49152;branch=z9hG4bK0ae0465e0000003c4e1bf7f7000053a90000001c");
    request.SetViaList(vias);
    request.SetContentType("application/sdp");
    string sdp = 
    "v=0"_CRLF
    "o=- 3519444599 3519444599 IN IP4 10.224.70.94"_CRLF
    "s=SJphone"_CRLF
    "c=IN IP4 10.224.70.94"_CRLF
    "t=0 0"_CRLF
    "m=audio 49156 RTP/AVP 3 97 98 8 0 101"_CRLF
    "c=IN IP4 10.224.70.94"_CRLF
    "a=rtpmap:3 GSM/8000"_CRLF
    "a=rtpmap:97 iLBC/8000"_CRLF
    "a=rtpmap:98 iLBC/8000"_CRLF
    "a=fmtp:98 mode=20"_CRLF
    "a=rtpmap:8 PCMA/8000"_CRLF
    "a=rtpmap:0 PCMU/8000"_CRLF
    "a=rtpmap:101 telephone-event/8000"_CRLF
    "a=fmtp:101 0-16"_CRLF
    "a=setup:active"_CRLF
    "a=sendrecv"_CRLF;
    request.SetSessionDescription(SessionDescriptionImpl::Create(sdp));
    */
    //request.SetBodyContent(vias);
    //EXPECT_TRUE(request.Sync2Resip());
    //cout << request.GetResipMessage()->getEncoded().c_str() << endl;
}


TEST_F (TestResipSipMessage, ReadWriteURI)
{
    SipFromImpl sipFrom("sip:msml@10.224.54.27:5060");

    EXPECT_TRUE(sipFrom.GetFromUri()->GetHost() == "10.224.54.27");
    EXPECT_TRUE(sipFrom.GetFromUri()->GetPort() == 5060);
    EXPECT_TRUE(sipFrom.GetFromUri()->GetScheme() == "sip");
    EXPECT_TRUE(sipFrom.GetFromUri()->GetUser() == "msml");

    sipFrom.GetFromUri()->SetHost("10.224.54.28");
    EXPECT_TRUE(sipFrom.GetFrom() == "sip:msml@10.224.54.28:5060");
    EXPECT_TRUE(sipFrom.toString() == "sip:msml@10.224.54.28:5060");
    
    sipFrom.GetFromUri()->SetPort(5061);
    EXPECT_TRUE(sipFrom.GetFrom() == "sip:msml@10.224.54.28:5061");
    EXPECT_TRUE(sipFrom.toString() == "sip:msml@10.224.54.28:5061");

    sipFrom.GetFromUri()->SetScheme("sips");
    EXPECT_TRUE(sipFrom.GetFrom() == "sips:msml@10.224.54.28:5061");
    EXPECT_TRUE(sipFrom.toString() == "sips:msml@10.224.54.28:5061");

    sipFrom.GetFromUri()->SetUser("sam");
    EXPECT_TRUE(sipFrom.GetFrom() == "sips:sam@10.224.54.28:5061");
    EXPECT_TRUE(sipFrom.toString() == "sips:sam@10.224.54.28:5061");

    SipToImpl sipTo("sip:msml@10.224.54.27:5060");

    EXPECT_TRUE(sipTo.GetToUri()->GetHost() == "10.224.54.27");
    EXPECT_TRUE(sipTo.GetToUri()->GetPort() == 5060);
    EXPECT_TRUE(sipTo.GetToUri()->GetScheme() == "sip");
    EXPECT_TRUE(sipTo.GetToUri()->GetUser() == "msml");

    sipTo.GetToUri()->SetHost("10.224.54.28");
    EXPECT_TRUE(sipTo.GetTo() == "sip:msml@10.224.54.28:5060");
    EXPECT_TRUE(sipTo.toString() == "sip:msml@10.224.54.28:5060");
    
    sipTo.GetToUri()->SetPort(5061);
    EXPECT_TRUE(sipTo.GetTo() == "sip:msml@10.224.54.28:5061");
    EXPECT_TRUE(sipTo.toString() == "sip:msml@10.224.54.28:5061");

    sipTo.GetToUri()->SetScheme("sips");
    EXPECT_TRUE(sipTo.GetTo() == "sips:msml@10.224.54.28:5061");
    EXPECT_TRUE(sipTo.toString() == "sips:msml@10.224.54.28:5061");

    sipTo.GetToUri()->SetUser("sam");
    EXPECT_TRUE(sipTo.GetTo() == "sips:sam@10.224.54.28:5061");
    EXPECT_TRUE(sipTo.toString() == "sips:sam@10.224.54.28:5061");
}

