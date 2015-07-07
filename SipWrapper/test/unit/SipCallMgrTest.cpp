#include "CxxTestDefs.h"
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "SipCallMgr.h"

class TESTSipCallMgrImpl : public ::testing::Test
{
public:
    TESTSipCallMgrImpl()
    {
    }
    
    ~TESTSipCallMgrImpl() 
    {
    }
    
    virtual void SetUp()
    {
    }
 
    virtual void TearDown()
    {
    }
    
};    

TEST_F (TESTSipCallMgrImpl, LookForLiveCall)
{

}

