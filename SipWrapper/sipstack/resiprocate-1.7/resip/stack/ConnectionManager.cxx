#if defined(HAVE_CONFIG_H)
#include "resip/stack/config.hxx"
#endif

#include "resip/stack/ConnectionManager.hxx"
#include "resip/stack/InteropHelper.hxx"
#include "rutil/Logger.hxx"
#include "rutil/Inserter.hxx"

#include <vector>

using namespace resip;
using namespace std;

#define RESIPROCATE_SUBSYSTEM Subsystem::TRANSPORT

UInt64 ConnectionManager::MinimumGcAge = 1;  // in milliseconds
bool ConnectionManager::EnableAgressiveGc = false;

ConnectionManager::ConnectionManager() : 
   mHead(0,Tuple(),0,Compression::Disabled),
   mWriteHead(ConnectionWriteList::makeList(&mHead)),
   mReadHead(ConnectionReadList::makeList(&mHead)),
   mLRUHead(ConnectionLruList::makeList(&mHead)),
   mFlowTimerLRUHead(FlowTimerLruList::makeList(&mHead)),
   mPollGrp(0)
{
   DebugLog(<<"ConnectionManager::ConnectionManager() called ");
}

ConnectionManager::~ConnectionManager()
{
   while (!mAddrMap.empty())
   {
      delete mAddrMap.begin()->second;
   }
   assert(mReadHead->empty());
   assert(mWriteHead->empty());
   assert(mLRUHead->empty());
   assert(mFlowTimerLRUHead->empty());
}

Connection*
ConnectionManager::findConnection(const Tuple& addr)
{
   if (addr.mFlowKey != 0)
   {
      IdMap::iterator i = mIdMap.find(addr.mFlowKey);
      if (i != mIdMap.end())
      {
         if(i->second->who()==addr)
         {
            DebugLog(<<"Found fd " << addr.mFlowKey);
            return i->second;
         }
         else
         {
            DebugLog(<<"fd " << addr.mFlowKey 
                     << " exists, but does not match the destination. FD -> "
                     << i->second->who() << ", tuple -> " << addr);
         }
      }
      else
      {
         DebugLog(<<"fd " << addr.mFlowKey << " does not exist.");
      }

      if(addr.onlyUseExistingConnection)
      {
         return 0;
      }
   }
   
   AddrMap::iterator i = mAddrMap.find(addr);
   if (i != mAddrMap.end())
   {
      DebugLog(<<"Found connection for tuple "<< addr );
      return i->second;
   }

   
   DebugLog(<<"Could not find a connection for " << addr);
   return 0;
}

const Connection* 
ConnectionManager::findConnection(const Tuple& addr) const
{
   if (addr.mFlowKey != 0)
   {
      IdMap::const_iterator i = mIdMap.find(addr.mFlowKey);
      if (i != mIdMap.end())
      {
         if(i->second->who()==addr)
         {
            DebugLog(<<"Found fd " << addr.mFlowKey);
            return i->second;
         }
         else
         {
            DebugLog(<<"fd " << addr.mFlowKey 
                     << " exists, but does not match the destination. FD -> "
                     << i->second->who() << ", tuple -> " << addr);
         }
      }
      else
      {
         DebugLog(<<"fd " << addr.mFlowKey << " does not exist.");
      }
   }
   
   AddrMap::const_iterator i = mAddrMap.find(addr);
   if (i != mAddrMap.end())
   {
      DebugLog(<<"Found connection for tuple "<< addr );
      return i->second;
   }

   
   DebugLog(<<"Could not find a connection for " << addr);
   return 0;
}

void
ConnectionManager::buildFdSet(FdSet& fdset)
{
   // If using PollGrp, caller shouldn't call this
   assert( mPollGrp==0 );

   for (ConnectionReadList::iterator i = mReadHead->begin(); 
        i != mReadHead->end(); ++i)
   {
      fdset.setRead((*i)->getSocket());
      fdset.setExcept((*i)->getSocket());
   }

   for (ConnectionWriteList::iterator i = mWriteHead->begin(); 
        i != mWriteHead->end(); ++i)
   {
      fdset.setWrite((*i)->getSocket());
      fdset.setExcept((*i)->getSocket());
   }
}

void
ConnectionManager::addToWritable(Connection* conn)
{
   if ( mPollGrp ) 
   {
      mPollGrp->modPollItem(conn->mPollItemHandle, FPEM_Read|FPEM_Write);
   } 
   else 
   {
      mWriteHead->push_back(conn);
   }
}

void
ConnectionManager::removeFromWritable(Connection* conn)
{
   if ( mPollGrp ) 
   {
      mPollGrp->modPollItem(conn->mPollItemHandle, FPEM_Read);
   }
   else
   {
      assert(!mWriteHead->empty());
      conn->ConnectionWriteList::remove();
   }
}

void
ConnectionManager::addConnection(Connection* connection)
{
   assert(mAddrMap.find(connection->who())==mAddrMap.end());

   //DebugLog (<< "ConnectionManager::addConnection() " << connection->mWho.mFlowKey  << ":" << connection->mSocket);
   
   
   mAddrMap[connection->who()] = connection;
   mIdMap[connection->who().mFlowKey] = connection;

   if ( mPollGrp ) 
   {
      connection->mPollItemHandle = mPollGrp->addPollItem(
         connection->getSocket(), FPEM_Read, connection);
   }
   else 
   {
      mReadHead->push_back(connection);
   }
   mLRUHead->push_back(connection);

   // Garbage collect old connections if agressive is enabled
   if(EnableAgressiveGc)
   {
      gc(MinimumGcAge, 0);  // cleanup all connections that haven't seen data in last x ms
   }

   //DebugLog (<< "count=" << mAddrMap.count(connection->who()) << "who=" << connection->who() << " mAddrMap=" << Inserter(mAddrMap));
   //assert(mAddrMap.begin()->first == connection->who());
   assert(mAddrMap.count(connection->who()) == 1);
}

void
ConnectionManager::removeConnection(Connection* connection)
{
   //DebugLog (<< "ConnectionManager::removeConnection()");

   mIdMap.erase(connection->mWho.mFlowKey);
   mAddrMap.erase(connection->mWho);

   if ( mPollGrp ) 
   {
      mPollGrp->delPollItem(connection->mPollItemHandle);
   }
   else
   {
      assert(!mReadHead->empty());
      connection->ConnectionReadList::remove();
      connection->ConnectionWriteList::remove();
      if(connection->isFlowTimerEnabled())
      {
         connection->FlowTimerLruList::remove();
      }
      else
      {
         connection->ConnectionLruList::remove();
      }
   }
}

// release excessively old connections (free up file descriptors)
void
ConnectionManager::gc(UInt64 relThreshold, unsigned int maxToRemove)
{
   UInt64 curTimeMs = Timer::getTimeMs();
   UInt64 threshold = curTimeMs - relThreshold;
   DebugLog(<< "recycling connections not used in last " << relThreshold/1000.0 << " seconds");

   // Look through non-flow-timer connections and close those using the passed in relThreshold
   unsigned int numRemoved = 0;
   for (ConnectionLruList::iterator i = mLRUHead->begin();
        i != mLRUHead->end() &&
        (maxToRemove == 0 || numRemoved != maxToRemove);)
   {
      if ((*i)->whenLastUsed() < threshold)
      {
         Connection* discard = *i;
         InfoLog(<< "recycling connection: " << discard << " " << discard->getSocket());
         // iterate before removing
         ++i;
         delete discard;
         numRemoved++;
      }
      else
      {
         break;
      }
   }

   // Look through flow-timer connections and close those using the configured FlowTimer 
   // value + the configured grace period as a threshold
   if(mFlowTimerLRUHead->begin() != mFlowTimerLRUHead->end())
   {
      threshold = curTimeMs - ((InteropHelper::getFlowTimerSeconds() + InteropHelper::getFlowTimerGracePeriodSeconds()) * 1000);
      for (FlowTimerLruList::iterator i2 = mFlowTimerLRUHead->begin();
         i2 != mFlowTimerLRUHead->end() &&
         (maxToRemove == 0 || numRemoved != maxToRemove);)
      {  
         if ((*i2)->whenLastUsed() < threshold)
         {
            Connection* discard = *i2;
            InfoLog(<< "recycling flow-timer enabled connection: " << discard << " " << discard->getSocket());
            // iterate before removing
            ++i2;
            delete discard;
            numRemoved++;
         }
         else
         {
            break;
         }
      }
   }
}

// move to youngest
void
ConnectionManager::touch(Connection* connection)
{
   connection->resetLastUsed();
   if(connection->isFlowTimerEnabled())
   {
      connection->FlowTimerLruList::remove();
      mFlowTimerLRUHead->push_back(connection);
   }
   else
   {
      connection->ConnectionLruList::remove();
      mLRUHead->push_back(connection);
   }
}

void 
ConnectionManager::moveToFlowTimerLru(Connection *connection)
{
   connection->ConnectionLruList::remove();
   mFlowTimerLRUHead->push_back(connection);
}

void
ConnectionManager::process(FdSet& fdset)
{
   assert( mPollGrp==NULL );	// owner shouldn't call this if polling

   // process the write list
   for (ConnectionWriteList::iterator writeIter = mWriteHead->begin();
        writeIter != mWriteHead->end(); )
   {
      Connection* currConnection = *writeIter;

      // update iterator to next first so that it can traverse safely
      // even if current one is removed from the list later
      ++writeIter;

      if (!currConnection)
         continue;

      if (fdset.readyToWrite(currConnection->getSocket()))
      {
         currConnection->performWrite();
      }
      else if (fdset.hasException(currConnection->getSocket()))
      {
         int errNum = 0;
         int errNumSize = sizeof(errNum);
         getsockopt(currConnection->getSocket(), SOL_SOCKET, SO_ERROR, (char *)&errNum, (socklen_t *)&errNumSize);
         InfoLog(<< "Exception writing to socket " << currConnection->getSocket() << " code: " << errNum << "; closing connection");
         delete currConnection;
      }
   }

   // process the read list
   for (ConnectionReadList::iterator readIter = mReadHead->begin();
        readIter != mReadHead->end(); )
   {
      Connection* currConnection = *readIter; 

      // update iterator to next first so that it can traverse safely
      // even if current one is removed from the list later
      ++readIter;

      if (!currConnection)
         continue;

      if ( fdset.readyToRead(currConnection->getSocket()) ||
           currConnection->hasDataToRead() )
      {
         fdset.clear(currConnection->getSocket());
         
         int bytesRead = currConnection->read();
         DebugLog(<< "ConnectionManager::process() " << " read=" << bytesRead);
         if (bytesRead < 0)
         {
            DebugLog(<< "Closing connection bytesRead=" << bytesRead);
            delete currConnection;
         }
      }
      else if (fdset.hasException(currConnection->getSocket()))
      {
         int errNum = 0;
         int errNumSize = sizeof(errNum);
         getsockopt(currConnection->getSocket(), SOL_SOCKET, SO_ERROR, (char *)&errNum, (socklen_t *)&errNumSize);
         InfoLog(<< "Exception reading from socket " << currConnection->getSocket() << " code: " << errNum << "; closing connection");
         delete currConnection;
      }
   }
}

void
ConnectionManager::setPollGrp(FdPollGrp *grp) 
{
    assert( mPollGrp == NULL );
    mPollGrp = grp;
}

/* ====================================================================
 * The Vovida Software License, Version 1.0 
 * 
 * Copyright (c)
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
 */