#include "CxxTestDefs.h"
#include <gtest/gtest.h>
#include <gmock/gmock.h>


#include "MediaDescriptionImpl.h"

class TESTSDCryptoParamImpl : public ::testing::Test
{
public:
    TESTSDCryptoParamImpl()
    {
    }
    
    ~TESTSDCryptoParamImpl() 
    {
    }
    
    virtual void SetUp()
    {
    }
 
    virtual void TearDown()
    {
    }
    
};    

TEST_F (TESTSDCryptoParamImpl, SetBase64Key)
{
    SDCryptoParamImpl crypto;
    crypto.SetBase64Key("d0RmdmcmVCspeEc3QGZiNWpVLFJhQX1cfHAwJSoj");

    cout << "crypto.GetMasterKey(): " << crypto.GetMasterKey() << endl;
    cout << "crypto.GetMasterSalt(): " << crypto.GetMasterSalt() << endl;
    EXPECT_TRUE(crypto.GetMasterKey().compare("wDfvg&T+)xG7@fb5") == 0);
    EXPECT_TRUE(crypto.GetMasterSalt().compare("jU,RaA}\\|p0%*#") == 0);


}
