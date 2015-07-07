#include "CxxTestDefs.h"
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "SipCallImpl.h"
#include "SipCallMgr.h"

class TESTSipCallKey : public ::testing::Test
{
public:
    TESTSipCallKey()
    {
    }
    
    ~TESTSipCallKey() 
    {
    }
    
    virtual void SetUp()
    {
    }
 
    virtual void TearDown()
    {
    }
    
};    

TEST_F (TESTSipCallKey, testKey)
{
    /*
    SipCallMgr::SipCallMap callMap;

    SipCallImpl* call = new SipCallImpl(NULL, NULL, "<sip:6434@10.224.54.213>;tag=b8f2ef1659d", "<sip:6435@10.224.54.213>", "1965D5556805462B939341359F1FFE9E0x0ae0463f");
    call->add_reference();
    call->SetCallState(CALL_STATE_START);

    SipCallKey key("1965D5556805462B939341359F1FFE9E0x0ae0463f", "b8f2ef1659d", "");
    callMap.insert(make_pair(key, call));

    SipCallKey key1("1965D5556805462B939341359F1FFE9E0x0ae0463f", "b8f2ef1659d", "sefsdfsdf");
    SipCallMgr::SipCallMap::iterator iter = callMap.find(key1);

    EXPECT_TRUE(iter != callMap.end());
    
    SipCallImpl* findCall = iter->second;
    EXPECT_TRUE(findCall->GetFrom() == "<sip:6434@10.224.54.213>;tag=b8f2ef1659d");


    SipCallKey key2("1965D5556805462B939341359F1FFE9E0x0ae0463f", "b8f2ef1659d", "");
    iter = callMap.find(key2);
    EXPECT_TRUE(iter != callMap.end());

    SipCallKey key3("1965D5556805462B939341359F1FFE9E0x0ae0463f", "b8f2ef1659d_", "");
    iter = callMap.find(key3);
    EXPECT_TRUE(iter == callMap.end());
    */
}



class TESTSipCallImpl : public ::testing::Test
{
public:
    TESTSipCallImpl()
    {
    }
    
    ~TESTSipCallImpl() 
    {
    }
    
    virtual void SetUp()
    {
    }
 
    virtual void TearDown()
    {
    }
    
};    

TEST_F (TESTSipCallImpl, Invite)
{

}

TEST_F (TESTSipCallImpl, Ringing)
{

}

// ... TBD
