#include <iostream>
#include <signal.h>
#include "sipsdk.h"

#include "SipUtil.h"
#include "SipTrace.h"

using namespace std;

bool g_stop = false;
void SIG_HANDLER(int signo)
{
    if (signo == SIGINT || signo == SIGTSTP )
        g_stop = true;
}

class CallMsg
{
public:
    CallMsg(ISipCall* call, ISipMessage* msg);
    virtual ~CallMsg();

    ISipCall* _call;
    ISipMessage* _msg;
};

CallMsg::CallMsg(ISipCall* call, ISipMessage* msg) : _call(call), _msg(msg)
{
    _call->AddRef();
    _msg->AddRef();
}

CallMsg::~CallMsg()
{
    _call->ReleaseRef();
    _msg->ReleaseRef();
}

class CallWorker : public ThreadIf
{
public:
    CallWorker();
    virtual ~CallWorker();
    static CallWorker* instance();
    
    void            Start();
    void            Stop();
    void            PushMsg(ISipCall * call, ISipMessage* msg);
    void            ProcessMsg();

    void            thread();
    //int             OnThreadRun();
    //void            OnThreadCleanup();
    //virtual         LPCSTR type_str() const { return "SIP_SDK_PROVIDER_THREAD"; }

private:
    static CallWorker* s_instance;
    
    list<CallMsg*>          mMsgQueue;
    Mutex                   mLock;
    bool                    mStopFlag;
};

CallWorker* CallWorker::s_instance = NULL;

CallWorker* CallWorker::instance()
{
    if(!s_instance)
        s_instance = new CallWorker;

    return s_instance;
}

CallWorker::CallWorker() : mMsgQueue(), mLock(), mStopFlag(true)
{
}

CallWorker::~CallWorker()
{
}

void CallWorker::thread()
{
    while(!isShutdown())
    {
        this->ProcessMsg();
        sleep(1);
    }
    

}

void CallWorker::Start()
{
    mStopFlag = false;
    this->run();
    
    return;
}

void CallWorker::Stop()
{
    mStopFlag = true;
    this->shutdown();
    this->join();
    return;
}

void CallWorker::ProcessMsg()
{
    
    Lock lock(mLock);

    list<CallMsg*>::iterator iter = mMsgQueue.begin();
    for(; iter != mMsgQueue.end(); ++ iter)
    {
        CallMsg * callMsg = *iter;
        
        // ...
        if(callMsg->_msg->IsSipRequest() && callMsg->_msg->GetRequstMethod() == SIP_METHOD_INVITE)
        {

            ISipMessage* response = ISipMessage::CreateSipResponse(callMsg->_msg);
            //response->add_reference();

            response->SetContentType("application/sdp");

            ISessionDescription* sdp = response->GetSessionDescription();

            // test sdp clone
            ISessionDescription* newsdp = sdp->Clone();
            delete  newsdp;

            cout << "old sdp: " << sdp << ", new sdp: " << newsdp << endl;
            
            sdp->GetOrigin()->SetAddress("10.224.70.63");
         
            sdp->SetInfo("info");
            sdp->SetSessionName("session");
            sdp->SetURI("uri");
            sdp->SetVersion(0);
            sdp->GetConnection()->SetAddress("10.224.70.63");

            ISessionDescription::MediaDescrContainer& mediaList = sdp->GetMediaDescriptionList();

            ISDMediaDescription* media = ISDMediaDescription::Create();
            //media->AddRef();
            media->SetInfo("audio");
            media->SetMediaPort(49152);
            media->SetMediaType("audio");
            media->SetProtocol("RTP/AVP");
            media->GetAttribute()->AddAttribute("test_attr", "test_value");
            

            vector<int>& format = media->GetPayLoadTypeList();
            format.push_back(3);
            
            vector<ISDCodec*>& codecs = media->GetAllCodecs();
            ISDCodec* codec = ISDCodec::Create();
            //codec->AddRef();
            
            codecs.push_back(codec);
            codec->SetName("GSM");
            codec->SetPayloadType(3);
            codec->SetRate(8000);

            mediaList.push_back(media);


            // check crypto
            SIPSDK_INFOTRACE("--------------------check crypto------------------------");
            ISessionDescription * sdp1 = callMsg->_msg->GetSessionDescription();
            if(sdp != NULL)
            {
                vector<ISDMediaDescription*> mediaList1 = sdp1->GetMediaDescriptionList();
                vector<ISDMediaDescription*>::iterator iter = mediaList1.begin();
                for(; iter != mediaList1.end(); ++ iter)
                {
                    ISDMediaDescription* mediaDesc = *iter;
                    vector<ISDCrypto*>& cryptoList = mediaDesc->GetAllCryptos();

                    vector<ISDCrypto*>::iterator iter1 = cryptoList.begin();
                    for(; iter1 != cryptoList.end(); ++ iter1)
                    {
                        ISDCrypto* crypto = *iter1;
                        SIPSDK_INFOTRACE("Tag: " << crypto->GetTag());
                        SIPSDK_INFOTRACE("CryptoSuiteType: " << crypto->GetCryptoSuiteType());

                        vector<ISDCryptoParam*> paramList = crypto->GetAllParams();
                        vector<ISDCryptoParam*>::iterator iter2 = paramList.begin();
                        for(; iter2 != paramList.end(); ++ iter2)
                        {
                            ISDCryptoParam* param = *iter2;
                            SIPSDK_INFOTRACE("GetBase64Key: " << param->GetBase64Key());

                            param->SetBase64Key(param->GetBase64Key());
                                
                            SIPSDK_INFOTRACE("GetLifeTime: " << param->GetLifeTime());
                            SIPSDK_INFOTRACE("GetMasterKey: " << param->GetMasterKey());
                            SIPSDK_INFOTRACE("GetMasterKeyIndex: " << param->GetMasterKeyIndex());
                            SIPSDK_INFOTRACE("GetMasterKeyIndexLen: " << param->GetMasterKeyIndexLen());
                            SIPSDK_INFOTRACE("GetMasterSalt: " << param->GetMasterSalt());
                            
                        }
                    }
                }
            }

            ISessionDescription * localSDP = sdp1->Clone();
            ISessionDescription * finalSDP = localSDP->Negotiate(sdp1);

            cout << "@@@@@@@@@@@@@@@@@ sdp: " << endl << finalSDP->toString() << endl;

            SIPSDK_SAFE_DELETE(localSDP);
            SIPSDK_SAFE_DELETE(finalSDP);
            // end
            
            callMsg->_call->OK(response);

            /*
            // for test, it is unreasonable
            ISipCall * newCall = ISipCall::CreateSipCall("sip:service@10.224.55.24:5061", "sip:sipp@10.224.55.26:5060");
            ISipMessage * inviteSeq = ISipMessage::CreateSipRequest();
            inviteSeq->SetRequestURI("sip:sipp@10.224.55.26:5060");
            newCall->Invite(inviteSeq);

            ISipMessage * ackSeq = ISipMessage::CreateSipRequest();
            newCall->ACK(ackSeq);
            */

        }
        else if(callMsg->_msg->IsSipRequest() && callMsg->_msg->GetRequstMethod() == SIP_METHOD_ACK)
        {
            ISipMessage* response = ISipMessage::CreateSipResponse(callMsg->_msg);
            callMsg->_call->OK(response);
        }
        else if(callMsg->_msg->IsSipRequest() && callMsg->_msg->GetRequstMethod() == SIP_METHOD_OPTIONS)
        {
            ISipMessage * response = ISipMessage::CreateSipResponse(callMsg->_msg);
            response->SetResponseStatusCode(200);
            callMsg->_call->Respond(response);

        }
        
        mMsgQueue.erase(iter);
        delete callMsg;
        
        break;
    }
}

void CallWorker::PushMsg(ISipCall * call, ISipMessage* msg)
{
    Lock lock(mLock);
    CallMsg * callMsg = new CallMsg(call, msg);
    mMsgQueue.push_back(callMsg);
}


class MySipCallSink : public ISipCallSink, public CReferenceControl
{
public:
    MySipCallSink() {}
    virtual ~MySipCallSink() {}
    
    void AddReference()
    {
        CReferenceControl::add_reference();
    }

    void ReleaseReference()
    {
        CReferenceControl::release_reference();
    }
    
    virtual CmResult OnRecvInviteRqst(ISipCall * call, ISipMessage * request)
    {
        cout << "OnRecvInviteRqst..." << endl;

        CallWorker::instance()->PushMsg(call, request);
        

        
        return 0;
    }

    virtual CmResult OnRecvTrying(ISipCall * call, ISipMessage * request)
    {
        cout << "OnRecvTrying..." << endl;
        return 0;     
    }
    
    virtual CmResult OnRecvRinging(ISipCall * call, ISipMessage * request)
    {
        cout << "OnRecvRinging..." << endl;
        return 0;     
    }
    
    virtual CmResult OnRecvInviteResp(ISipCall * call, ISipMessage* response)
    {
        cout << "OnRecvInviteResp..." << endl;
        return 0;
    }
    
    virtual CmResult OnRecvSubscribeRqst(ISipCall * call, ISipMessage * request)
    {
        cout << "OnRecvSubscribeRqst..." << endl;
        return 0;
    }
    
    virtual CmResult OnRecvNotifyRqst(ISipCall * call, ISipMessage * request)
    {
        cout << "OnRecvNotifyRqst..." << endl;
        return 0;
    }
    
    virtual CmResult OnRecvACKRqst(ISipCall * call, ISipMessage * request)
    {
        cout << "OnRecvACKRqst..." << endl;
        return 0;
    }

    virtual CmResult OnRecvByeRqst(ISipCall * call, ISipMessage * request)
    {
        cout << "OnRecvByeRqst..." << endl;
        Expert::OnRecvByeRqst_u(call, request);
        return 0;
    }
    
    virtual CmResult OnRecvInfoRqst(ISipCall * call, ISipMessage * request)
    {
        cout << "OnRecvInfoRqst..." << endl;
        return 0;
    }
    
    virtual CmResult OnRecvOptionsRqst(ISipCall * call, ISipMessage * request)
    {
        cout << "OnRecvOptionsRqst..." << endl;
        Expert::OnRecvOptionsRqst_u(call, request);
        return 0;
    }
    
    virtual CmResult OnRecvResp(ISipCall * call, ISipMessage * response)
    {
        cout << "OnRecvResp..." << endl;
        return 0;
    }
    
    virtual CmResult OnRecvCancelRqst(ISipCall * call, ISipMessage * response)
    {
        cout << "OnRecvCancelRqst..." << endl;
        return 0;
    }
    
	virtual CmResult OnSessionTimeout()
	{
	    cout << "OnSessionTimeout..." << endl;
	    return 0;
	}
};

class MySipProviderSink : public ISipProviderSink, public CReferenceControl
{
public:
    MySipProviderSink() {}
    virtual ~MySipProviderSink() {}
    
    void AddReference()
    {
        CReferenceControl::add_reference();
    }

    void ReleaseReference()
    {
        CReferenceControl::release_reference();
    }
    
    CmResult OnCallIndication(ISipCall * call)
    {
        MySipCallSink * sink = new MySipCallSink();


        call->SetSipCallSink(sink);
        
        return 0;
    }
};

class MyLoggerSink : public ISipLoggerSink, public CReferenceControl
{
public:
    MyLoggerSink() {}
    virtual ~MyLoggerSink() {}
    
    void AddReference()
    {
        CReferenceControl::add_reference();
    }

    void ReleaseReference()
    {
        CReferenceControl::release_reference();
    }
    
    virtual void    Detail(const char* str) { cout << "DETAIL: " << str << endl;}
    virtual void    Info(const char* str) {cout << "INFO: " << str << endl;}
    virtual void    Warning(const char* str) {cout << "WARNING: " << str << endl;} 
    virtual void    Error(const char* str) {cout << "ERROR: " << str << endl;}
};

int main()
{
    cout << "SipSDK application demo ..." << endl;    

    /*
    ISipMessage * request = ISipMessage::CreateSipRequest();
    cout << "1: " << request->GetRequestURI() << endl;
    request->SetRequestURI("test");
    cout << "2: " << request->GetRequestURI() << endl;


    cout << "method1: " << request->GetRequstMethod() << endl;
    request->SetRequstMethod(SIP_METHOD_ACK);
    cout << "method2: " << request->GetRequstMethod() << endl;
    */
    
    struct sigaction sa;
    {
        sa.sa_handler = SIG_HANDLER;

        sigemptyset(&sa.sa_mask);
        sigaddset(&sa.sa_mask, SIGSEGV);
        sa.sa_flags = SA_RESTART;
    }
    
    ::sigaction(SIGINT, &sa, NULL);
    ::sigaction(SIGTSTP, &sa, NULL);

    ISipProviderSink * providerSink = new MySipProviderSink;
    providerSink->AddReference();
    MyLoggerSink* myLogger = new MyLoggerSink;
    myLogger->AddReference();
    
    ISipProvider * instance = ISipProvider::GetInstance();
    
    ::SetupSipSDKLoggerSink(myLogger);

    instance->SetStackLogPath("./log", "mma_sip", STACK_LOG_DETAIL);
        
    instance->SetupSessionTimer(true, 1200, 360);
    instance->SetTlsCertPath(".");
    instance->SetTlsDomain("example.com");

    bool addFlag = instance->AddTransport(SIP_TRANSPORT_UDP, "10.224.55.24", 5061);
    if(!addFlag)
    {
        cout << "Add transport failed !!!!!!!!!!!!!!!!!" << endl;
        return 1;
    }
    
    instance->SetSipProviderSink(providerSink);

    
    instance->Start();
    //CallWorker::instance()->Start();

    
    while(!g_stop)
        usleep(100);


    
    // Query stack statistics data
    StackStat* stat = instance->GetStackStat();
    cout << "RECEIVE_REQUEST_NUM: " << stat->GetValue(StackStat::RECEIVE_REQUEST_NUM) << endl;


    // Program exist, destroy SDK resources
    instance->Stop();
    ISipProvider::DestroyInstance();
    providerSink->ReleaseReference();
    //CallWorker::instance()->Stop();
    ::RemoveSipSDKLoggerSink();
    
    return 0;
}

