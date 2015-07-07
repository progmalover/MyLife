#include "CxxTestDefs.h"
#include <gtest/gtest.h>
#include <gmock/gmock.h>


#include "SessionDescriptionImpl.h"

class TESTSessionDescriptionImpl : public ::testing::Test
{
public:
    TESTSessionDescriptionImpl()
    {
    }
    
    ~TESTSessionDescriptionImpl() 
    {
    }
    
    virtual void SetUp()
    {
    }
 
    virtual void TearDown()
    {
    }
    
};    

TEST_F (TESTSessionDescriptionImpl, Negotiate1)
{
    // remote and local sdp have the same content
    string remoteTxt("v=0\r\n"  
                    "o=- 333525334858460 333525334858460 IN IP4 192.168.0.156\r\n"
                    "s=test123\r\n"
                    "c=IN IP4 192.168.0.156\r\n"
                    "t=4058038202 0\r\n"
                    "m=audio 41466 RTP/AVP 101 0\r\n"
                    "a=ptime:20\r\n"
                    "a=rtpmap:0 PCMU/8000\r\n"
                    "a=rtpmap:101 telephone-event/8000\r\n"
                    "a=fmtp:101 0-11\r\n");
    ISessionDescription* remoteSdp = SessionDescriptionImpl::Create(remoteTxt); 
    cout << "remoteSdp: " << remoteSdp->toString() << endl;
    
    string localTxt("v=0\r\n"  
                    "o=- 333525334858460 333525334858460 IN IP4 192.168.0.156\r\n"
                    "s=test123\r\n"
                    "c=IN IP4 192.168.0.156\r\n"
                    "t=4058038202 0\r\n"
                    "m=audio 41466 RTP/AVP 101 0\r\n"
                    "a=ptime:20\r\n" 
                    "a=rtpmap:0 PCMU/8000\r\n"
                    "a=rtpmap:101 telephone-event/8000\r\n"
                    "a=fmtp:101 0-11\r\n");
    ISessionDescription* localSdp = SessionDescriptionImpl::Create(localTxt);
    cout << "localSdp: " << localSdp->toString() << endl;
    
    ISessionDescription* finalSdp = localSdp->Negotiate(remoteSdp);
    EXPECT_TRUE(finalSdp != NULL);

    if(finalSdp)
        cout << "finalSdp: " << finalSdp->toString() << endl;
    
    SIPSDK_SAFE_DELETE(remoteSdp);
    SIPSDK_SAFE_DELETE(localSdp);
    SIPSDK_SAFE_DELETE(finalSdp);

}

TEST_F (TESTSessionDescriptionImpl, Negotiate2)
{
    // remote sdp has no "m=" row
    string remoteTxt("v=0\r\n"  
                    "o=- 333525334858460 333525334858460 IN IP4 192.168.0.156\r\n"
                    "s=test123\r\n"
                    "c=IN IP4 192.168.0.156\r\n"
                    "t=4058038202 0\r\n");
    ISessionDescription* remoteSdp = SessionDescriptionImpl::Create(remoteTxt); 
    cout << "remoteSdp: " << remoteSdp->toString() << endl;
    
    string localTxt("v=0\r\n"  
                    "o=- 333525334858460 333525334858460 IN IP4 192.168.0.156\r\n"
                    "s=test123\r\n"
                    "c=IN IP4 192.168.0.156\r\n"
                    "t=4058038202 0\r\n"
                    "m=audio 41466 RTP/AVP 0 101\r\n"
                    "a=ptime:20\r\n" 
                    "a=rtpmap:0 PCMU/8000\r\n"
                    "a=rtpmap:101 telephone-event/8000\r\n"
                    "a=fmtp:101 0-11\r\n");
    ISessionDescription* localSdp = SessionDescriptionImpl::Create(localTxt);
    cout << "localSdp: " << localSdp->toString() << endl;
    
    ISessionDescription* finalSdp = localSdp->Negotiate(remoteSdp);
    EXPECT_TRUE(finalSdp != NULL);

    if(finalSdp)
        cout << "finalSdp: " << finalSdp->toString() << endl;
    
    SIPSDK_SAFE_DELETE(remoteSdp);
    SIPSDK_SAFE_DELETE(localSdp);
    SIPSDK_SAFE_DELETE(finalSdp);

}

TEST_F (TESTSessionDescriptionImpl, Negotiate3)
{
    // remote sdp and local sdp has matrix codes
    string remoteTxt("v=0\r\n"
                   "o=anonymous 1076575175 1076575175 IN IP4 192.168.1.100\r\n"
                   "s=eConf 4.0\r\n"
                   "i=eConf 4.0\r\n"
                   "b=AS:256\r\n"
                   "t=0 0\r\n"
                   "m=audio 6000 RTP/AVP 102 104 9 4 0 8 98\r\n"
                   "a=fmtp:98 0-15\r\n"
                   "a=rtpmap:102 X-G72x1_/16000\r\n"
                   "a=rtpmap:104 X-G72x24_/16000\r\n"
                   "a=rtpmap:9 G722_/8000\r\n"
                   "a=rtpmap:4 G723_/8000\r\n"
                   "a=rtpmap:0 PCMU/8000\r\n"
                   "a=rtpmap:8 PCMA/8000\r\n"
                   "a=rtpmap:98 telephone-event_/8000\r\n"
                   "a=sendrecv\r\n"
                   "m=video 6002 RTP/AVP 97 98 34 31\r\n"
                   "b=AS:192\r\n"
                   "a=fmtp:97 QCIF=1/MaxBR=1920/\r\n"
                   "a=framerate:25.0\r\n"
                   "a=fmtp:34 QCIF=1/MaxBR=1920\r\n"
                   "a=fmtp:31 QCIF=1/MaxBR=1920\r\n"
                   "a=rtpmap:97 H263-1998/90000\r\n"
                   "a=rtpmap:98 MP4V-ES/90000\r\n"
                   "a=rtpmap:34 H263/90000\r\n"
                   "a=rtpmap:31 H261/90000\r\n"
                   "a=sendrecv\r\n");
    ISessionDescription* remoteSdp = SessionDescriptionImpl::Create(remoteTxt); 
    cout << "remoteSdp: " << remoteSdp->toString() << endl;
    
    string localTxt("v=0\r\n"
                   "o=anonymous 1076575175 1076575175 IN IP4 192.168.1.100\r\n"
                   "s=eConf 4.0\r\n"
                   "i=eConf 4.0\r\n"
                   "b=AS:256\r\n"
                   "t=0 0\r\n"
                   "m=audio 6000 RTP/AVP 102 104 9 4 0 8 98\r\n"
                   "a=fmtp:98 0-15\r\n"
                   "a=rtpmap:102 X-G72x1/16000\r\n"
                   "a=rtpmap:104 X-G72x24/16000\r\n"
                   "a=rtpmap:9 G722/8000\r\n"
                   "a=rtpmap:4 G723/8000\r\n"
                   "a=rtpmap:0 PCMU/8000\r\n"
                   "a=rtpmap:8 PCMA/8000\r\n"
                   "a=rtpmap:98 telephone-event/8000\r\n"
                   "a=sendrecv\r\n"
                   "m=video 6002 RTP/AVP 97 98 34 31\r\n"
                   "b=AS:192\r\n"
                   "a=fmtp:97 QCIF=1/MaxBR=1920/\r\n"
                   "a=framerate:25.0\r\n"
                   "a=fmtp:34 QCIF=1/MaxBR=1920\r\n"
                   "a=fmtp:31 QCIF=1/MaxBR=1920\r\n"
                   "a=rtpmap:97 H263-1998_/90000\r\n"
                   "a=rtpmap:98 MP4V-ES_/90000\r\n"
                   "a=rtpmap:34 H263/90000\r\n"
                   "a=rtpmap:31 H261_/90000\r\n"
                   "a=sendrecv\r\n");
    ISessionDescription* localSdp = SessionDescriptionImpl::Create(localTxt);
    cout << "localSdp: " << localSdp->toString() << endl;
    
    ISessionDescription* finalSdp = localSdp->Negotiate(remoteSdp);
    EXPECT_TRUE(finalSdp != NULL);

    if(finalSdp)
        cout << "finalSdp: " << finalSdp->toString() << endl;
    
    SIPSDK_SAFE_DELETE(remoteSdp);
    SIPSDK_SAFE_DELETE(localSdp);
    SIPSDK_SAFE_DELETE(finalSdp);

}

TEST_F (TESTSessionDescriptionImpl, Negotiate4)
{
    // remote sdp and local sdp has no matrix codes
    string remoteTxt("v=0\r\n"
                   "o=anonymous 1076575175 1076575175 IN IP4 192.168.1.100\r\n"
                   "s=eConf 4.0\r\n"
                   "i=eConf 4.0\r\n"
                   "b=AS:256\r\n"
                   "t=0 0\r\n"
                   "m=audio 6000 RTP/AVP 102 104 9 4 0 8 98\r\n"
                   "a=fmtp:98 0-15\r\n"
                   "a=rtpmap:102 X-G72x1_/16000\r\n"
                   "a=rtpmap:104 X-G72x24_/16000\r\n"
                   "a=rtpmap:9 G722_/8000\r\n"
                   "a=rtpmap:4 G723_/8000\r\n"
                   "a=rtpmap:0 PCMU_/8000\r\n"
                   "a=rtpmap:8 PCMA_/8000\r\n"
                   "a=rtpmap:98 telephone-event_/8000\r\n"
                   "a=sendrecv\r\n"
                   "m=video 6002 RTP/AVP 97 98 34 31\r\n"
                   "b=AS:192\r\n"
                   "a=fmtp:97 QCIF=1/MaxBR=1920/\r\n"
                   "a=framerate:25.0\r\n"
                   "a=fmtp:34 QCIF=1/MaxBR=1920\r\n"
                   "a=fmtp:31 QCIF=1/MaxBR=1920\r\n"
                   "a=rtpmap:97 H263-1998/90000\r\n"
                   "a=rtpmap:98 MP4V-ES/90000\r\n"
                   "a=rtpmap:34 H263/90000\r\n"
                   "a=rtpmap:31 H261/90000\r\n"
                   "a=sendrecv\r\n");
    ISessionDescription* remoteSdp = SessionDescriptionImpl::Create(remoteTxt); 
    cout << "remoteSdp: " << remoteSdp->toString() << endl;
    
    string localTxt("v=0\r\n"
                   "o=anonymous 1076575175 1076575175 IN IP4 192.168.1.100\r\n"
                   "s=eConf 4.0\r\n"
                   "i=eConf 4.0\r\n"
                   "b=AS:256\r\n"
                   "t=0 0\r\n"
                   "m=audio 6000 RTP/AVP 102 104 9 4 0 8 98\r\n"
                   "a=fmtp:98 0-15\r\n"
                   "a=rtpmap:102 X-G72x1/16000\r\n"
                   "a=rtpmap:104 X-G72x24/16000\r\n"
                   "a=rtpmap:9 G722/8000\r\n"
                   "a=rtpmap:4 G723/8000\r\n"
                   "a=rtpmap:0 PCMU/8000\r\n"
                   "a=rtpmap:8 PCMA/8000\r\n"
                   "a=rtpmap:98 telephone-event/8000\r\n"
                   "a=sendrecv\r\n"
                   "m=video 6002 RTP/AVP 97 98 34 31\r\n"
                   "b=AS:192\r\n"
                   "a=fmtp:97 QCIF=1/MaxBR=1920/\r\n"
                   "a=framerate:25.0\r\n"
                   "a=fmtp:34 QCIF=1/MaxBR=1920\r\n"
                   "a=fmtp:31 QCIF=1/MaxBR=1920\r\n"
                   "a=rtpmap:97 H263-1998_/90000\r\n"
                   "a=rtpmap:98 MP4V-ES_/90000\r\n"
                   "a=rtpmap:34 H263_/90000\r\n"
                   "a=rtpmap:31 H261_/90000\r\n"
                   "a=sendrecv\r\n");
    ISessionDescription* localSdp = SessionDescriptionImpl::Create(localTxt);
    cout << "localSdp: " << localSdp->toString() << endl;
    
    ISessionDescription* finalSdp = localSdp->Negotiate(remoteSdp);
    EXPECT_TRUE(finalSdp == NULL);

    
    SIPSDK_SAFE_DELETE(remoteSdp);
    SIPSDK_SAFE_DELETE(localSdp);
    SIPSDK_SAFE_DELETE(finalSdp);

}

TEST_F (TESTSessionDescriptionImpl, Negotiate5)
{
    // remote sdp and local sdp has no matrix codes
    string remoteTxt("v=0\r\n"
                   "o=anonymous 1076575175 1076575175 IN IP4 192.168.1.100\r\n"
                   "s=eConf 4.0\r\n"
                   "i=eConf 4.0\r\n"
                   "b=AS:256\r\n"
                   "t=0 0\r\n"
                   "m=audio 6000 RTP/SAVP 0\r\n" 
                   "a=rtpmap:0 PCMU_/8000\r\n"
                   "a=crypto:1 AES_CM_128_HMAC_SHA1_32 inline:d0RmdmcmVCspeEc3QGZiNWpVLFJhQX1cfHAwJSoj|1:32\r\n");
    ISessionDescription* remoteSdp = SessionDescriptionImpl::Create(remoteTxt); 
    cout << "remoteSdp: " << remoteSdp->toString() << endl; 
    
    string localTxt("v=0\r\n"
                   "o=anonymous 1076575175 1076575175 IN IP4 192.168.1.100\r\n"
                   "s=eConf 4.0\r\n"
                   "i=eConf 4.0\r\n"
                   "b=AS:256\r\n"
                   "t=0 0\r\n"
                   "m=audio 6000 RTP/SAVP 0\r\n"
                   "a=rtpmap:0 PCMU_/8000\r\n"
                   "a=crypto:2 AES_CM_128_HMAC_SHA1_32 inline:d0RmdmcmVCspeEc3QGZiNWpVLFJhQX1cfHAwJSoj|1:32\r\n");
    ISessionDescription* localSdp = SessionDescriptionImpl::Create(localTxt);
    cout << "localSdp: " << localSdp->toString() << endl;
    
    ISessionDescription* finalSdp = localSdp->Negotiate(remoteSdp);
    cout << "finalSdp: " << finalSdp->toString() << endl;
    EXPECT_TRUE(finalSdp != NULL);

    
    SIPSDK_SAFE_DELETE(remoteSdp);
    SIPSDK_SAFE_DELETE(localSdp);
    SIPSDK_SAFE_DELETE(finalSdp);

}

