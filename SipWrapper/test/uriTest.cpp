#include <iostream>
#include <signal.h>
#include "sipsdk.h"

#include "SipUtil.h"
#include "SipTrace.h"
#define CCmString string

#define p_mytest "p_mytest_value"

using namespace std;

bool g_stop = false;
void SIG_HANDLER(int signo)
{
    if (signo == SIGINT || signo == SIGTSTP )
        g_stop = true;
}

/*
void TestSetParam(Uri & uri, string name, string value)
{
    Data name_data(name);
    
    ParameterTypes::Type param_type = ParameterTypes::getType(name_data.data(), (unsigned int)name_data.size());
    if(param_type == ParameterTypes::UNKNOWN)
        uri.param(UnknownParameterType(name_data)) = Data(value);
    else
    {
        Parameter* parameter = uri.getParameterByEnum(param_type);
        cout << parameter->getName() << endl;
    }
    
    
}*/

void Dump(ISipUri * uri)
{
    cout << "---------------------------" << endl;
    cout << uri->toString() << endl;
    cout << "  GetScheme: " << uri->GetScheme() << endl;
    cout << "    GetUser: " << uri->GetUser() << endl;
    cout << "GetPassword: " << uri->GetPassword() << endl;
    cout << "    GetHost: " << uri->GetHost() << endl;
    cout << "    GetPort: " << uri->GetPort() << endl;

    if(uri->HasParam("user"))
        cout << "     p_user: " << uri->GetDataParam("user") << endl;

    if(uri->HasParam("transport"))
        cout << "p_transport: " << uri->GetDataParam("transport") << endl;
    
    if(uri->HasParam("lr"))    
    cout << "       p_lr: " << uri->GetDataParam("lr") << endl;
    
    
     if(uri->HasParam("comp"))
    cout << "     p_comp: " << uri->GetDataParam("comp") << endl;

    if(uri->HasParam("ttl"))
    cout << "       p_ttl: " << uri->GetDataParam("ttl") << endl;

    if(uri->HasParam("method"))
    cout << "   p_method: " << uri->GetDataParam("method") << endl;

    if(uri->HasParam("maddr"))
    cout << "    p_maddr: " << uri->GetDataParam("maddr") << endl;
    
   if(uri->HasParam("AttendeeID"))
        cout << " AttendeeID: " << uri->GetUnknownParam("AttendeeID") << endl;

    cout << endl;
}


int main()
{
    ISipMessage * msg = ISipMessage::CreateSipRequest();
    msg->AddRef();
    
    msg->SetAlso("sip:303@10.0.0.15");
    {
        string s = msg->GetAlso();
        
        ISipUri * uri = ISipUri::newInstance();
        
        uri->AddRef();
        if(uri->fromString(s))
            cout << uri->toString() << endl;

        uri->ReleaseRef();
    }
    
    msg->ReleaseRef();
    
    /*string s("sip:alice@example.com:9702;transport=udp");
    ISipContact* pContact = ISipContact::newInstance();
    pContact->AddRef();

 
    ISipUri* pUri = pContact->GetContactUri();
    if (pUri == NULL)
    {
        pContact->ReleaseRef();
        return -1;
    }
    
    //pUri->fromString(s);
    cout << pUri->toString() << endl;
    
    pUri->SetScheme("sip");
    pUri->SetHost("example.com");
    pUri->SetPort(9702);
    pUri->SetUser("alice");
    pUri->SetPassword("pass");
    pUri->SetDataParam("user", "aliceX");
    Dump(pUri);
    
    pUri->SetDataParam("transport", "udp");
    Dump(pUri);
    
    pUri->SetDataParam("AttendeeID", "SSS_AttendeeID");
    Dump(pUri);
    
    pUri->SetDataParam("ttl", "124");
    Dump(pUri);
    
    pUri->SetUnknownParam("AttendeeID", "12");
    Dump(pUri);
    
    pUri->SetUnknownParam("transport", "mmm_Transport");
    Dump(pUri);
    
    pUri->SetDataParam("lr", "true");
    Dump(pUri);
   
    ContactList SContactList;
    SContactList.push_back(pContact);
    
    //ISipMessage * request = ISipMessage::CreateSipRequest();
    //request->AddRef();
    //request->SetContactList(SContactList);
    //request->ReleaseRef();
    
    pContact->SetDisplayName("Ricky Wang");
    pContact->SetQValue(0.7);
    pContact->SetExpires(30);
    
    string contact = pContact->toString();
    
    cout << "  toString: " << contact   << endl;
    cout << "GetContact: " << pContact->GetContact() << endl;
    
    pContact->ReleaseRef();
    
    ISipContact* pContactN = ISipContact::newInstance();
    pContactN->AddRef();
    pContactN->fromString(contact);
    cout << "  displayName: " << pContactN->GetDisplayName()   << endl;
    cout << "  GetQValue: "  << pContactN->GetQValue()  << ", HasQValue: " << pContactN->HasQValue() << endl;
    cout << "  GetExpires: " << pContactN->GetExpires() << ", HasExpires: " << pContactN->HasExpires()    << endl;
    cout << "  GetContact: " << pContactN->GetContact()  << endl;
    
    NameAddr na;
    static_cast<SipContactImpl*>(pContactN)->toNameAddr(na);    
    na.uri().param(p_transport) = "tls";
    stringstream oss;
    na.encodeParsed(oss);
    
    cout << "oss: " << oss.str() << endl;

    ISipUri * uri = pContactN->GetContactUri();
    Dump(uri);
    pContactN->ReleaseRef();
    */
    /*
    CCmString strScheme("sip");
    pUri->SetScheme(strScheme);
    
    CCmString strUser("user");
    pUri->SetUser(strUser);
    pUri->SetHost("10.224.42.24");
    pUri->SetPort(22);

    CCmString strParamUser("AttendeeID");
    pUri->SetParam(strParamUser, "mmp_hybrid");
    
    pContact->ReleaseRef();
    
    Uri uri("sip:alice@example.com;user=XXX;AttendeeID=YYYY;transport=ZZZ");
    TestSetParam(uri, "user", "XXS");
    */
    return 0;
}

