#if !defined(DUM_CommandLineParser_hxx)
#define DUM_CommandLineParser_hxx

#include <vector>
#include "resip/stack/Uri.hxx"
#include "rutil/Data.hxx"

namespace resip
{

class CommandLineParser
{
   public:
      CommandLineParser(int argc, char** argv);
      static resip::Uri toUri(const char* input, const char* description);
      static std::vector<resip::Data> toVector(const char* input, const char* description);
      
      Data mLogType;
      Data mLogLevel;
      Data mTlsDomain;
      Data mEnumSuffix;
      bool mForceRecordRoute;
      bool mAssumePath;
      resip::Uri mRecordRoute;
      int mUdpPort;
      int mTcpPort;
      int mTlsPort;
      int mDtlsPort;
      bool mUseV4;
      bool mUseV6;
      std::vector<Data> mDomains;
      std::vector<Data> mInterfaces;
      std::vector<Data> mRouteSet;
      Data mCertPath;
      Data mDbPath;
      bool mNoChallenge;
      bool mNoAuthIntChallenge;
      bool mRejectBadNonces;
      bool mNoWebChallenge;
      bool mNoRegistrar;
      bool mNoIdentityHeaders;
      bool mCertServer;
      Data mRequestProcessorChainName;
      Data mMySqlServer;
      Data mHttpHostname;
      int mHttpPort;
      bool mRecursiveRedirect;
      bool mDoQValue;
      Data mForkBehavior;
      bool mCancelBetweenForkGroups;
      bool mWaitForTerminate;
      int mMsBetweenForkGroups;
      int mMsBeforeCancel;
      bool mAllowBadReg;
      bool mParallelForkStaticRoutes;
      int mTimerC;
      Data mAdminPassword;
      Data mRegSyncPeerAddress;
      int mXmlRpcPort;
      Data mServerText;
      bool mUseInternalEPoll;
      bool mUseEventThread;
};
 
}

#endif

/* ====================================================================
 * The Vovida Software License, Version 1.0 
 * 
 * Copyright (c) 2000
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 
 * 3. The names "VOCAL", "Vovida Open Communication Application Library",
 *    and "Vovida Open Communication Application Library (VOCAL)" must
 *    not be used to endorse or promote products derived from this
 *    software without prior written permission. For written
 *    permission, please contact vocal@vovida.org.
 *
 * 4. Products derived from this software may not be called "VOCAL", nor
 *    may "VOCAL" appear in their name, without prior written
 *    permission of Vovida Networks, Inc.
 * 
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESSED OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, TITLE AND
 * NON-INFRINGEMENT ARE DISCLAIMED.  IN NO EVENT SHALL VOVIDA
 * NETWORKS, INC. OR ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT DAMAGES
 * IN EXCESS OF $1,000, NOR FOR ANY INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 * 
 * ====================================================================
 * 
 * This software consists of voluntary contributions made by Vovida
 * Networks, Inc. and many individuals on behalf of Vovida Networks,
 * Inc.  For more information on Vovida Networks, Inc., please see
 * <http://www.vovida.org/>.
 *
 * vi: set shiftwidth=3 expandtab:
 */

