/*------------------------------------------------------*/
/* Sip utility for internal usage                       */
/*                                                      */
/* CSipDef.h              `                             */
/*                                                      */
/* Copyright (C) WebEx Communications Inc.              */
/* All rights reserved                                  */
/*                                                      */
/*------------------------------------------------------*/

#ifndef _SIP_UTIL_H__
#define _SIP_UTIL_H__

#include <algorithm>
#include <list>
#include <set>
using namespace std;
//#include <WMUtilBase.h>

#include "resip/stack/SipStack.hxx"
#include "resip/stack/SipMessage.hxx"
#include "resip/stack/Helper.hxx"
#include "resip/stack/Uri.hxx"
#include "resip/stack/Helper.hxx"
#include "resip/stack/SdpContents.hxx"
#include "resip/stack/test/TestSupport.hxx"
#include "resip/stack/PlainContents.hxx"
#include "resip/stack/UnknownHeaderType.hxx"
#include "resip/stack/UnknownParameterType.hxx"
#include "resip/stack/EventStackThread.hxx"

#include "resip/stack/ssl/Security.hxx"

#include "rutil/Logger.hxx"
#include "rutil/ParseBuffer.hxx"
#include "rutil/Inserter.hxx"
#include "rutil/DataStream.hxx"
#include "rutil/ThreadIf.hxx"
#include "rutil/Timer.hxx"
#include "rutil/RWMutex.hxx"
#include "rutil/Mutex.hxx"
#include "rutil/FdPoll.hxx"
#include "rutil/Coders.hxx"

#include "resip/recon/sdp/SdpHelperResip.hxx"

using namespace resip;

#include "sipdef.h"

typedef enum
{
    SIPSDK_UAC,
    SIPSDK_UAS,
    SIPSDK_UA_UNKNOWN
} SIPSDK_UA_Type;

typedef struct
{
    bool   enabled;
    uint32 sessExpires;
    uint32 minSessExpires;
} SessionTimerParam;


struct SIPSDKDeleteObj
{
    template<typename T>               void operator()(T * ptr);
    template<typename T1, typename T2> void operator()(pair<T1, T2 *>   & pair);
    template<typename T1, typename T2> void operator()(pair<T1 * const, T2 *> & pair);
};

template<typename T>
void SIPSDKDeleteObj::operator()(T * ptr)
{
    try { delete ptr; } catch(...) { }
}

template< typename T1, typename T2 >
void SIPSDKDeleteObj::operator()(pair<T1, T2 *> & pair)
{
    try { delete pair.second; } catch(...) { }
}

template< typename T1, typename T2 >
void SIPSDKDeleteObj::operator()(pair<T1 * const, T2 *> & pair)
{
    try { delete pair.first;  } catch(...) { }
    try { delete pair.second; } catch(...) { }
}


struct SIPSDKReleaseObj
{
    template<typename T>                 void operator()(T * ptr);
    template< typename T1, typename T2 > void operator()(pair<T1,   T2 *> & pair);
    template< typename T1, typename T2 > void operator()(pair<T1 * const, T2 *> & pair);
};

template<typename T>
void SIPSDKReleaseObj::operator()(T * ptr)
{
    try { if(ptr) ptr->ReleaseRef(); } catch(...) { }
}

template< typename T1, typename T2 >
void SIPSDKReleaseObj::operator()(pair<T1, T2 *> & pair)
{
    try { if(pair.second) pair.second->ReleaseRef(); } catch(...) { }
}

template< typename T1, typename T2 >
void SIPSDKReleaseObj::operator()(pair<T1 * const, T2 *> & pair)
{
    try { if(pair.first) pair.first->ReleaseRef();   } catch(...) { }
    try { if(pair.second) pair.second->ReleaseRef(); } catch(...) { }
}

template<class T>
class SIPSDK_AUTO_REF
{
public:
    explicit SIPSDK_AUTO_REF(T * p = NULL) throw();
    SIPSDK_AUTO_REF(SIPSDK_AUTO_REF & )     throw();
    ~SIPSDK_AUTO_REF();
    
    void reset(T * p = NULL)  throw();
    T *  get()          const throw();
    T *  release()            throw();
    T &  operator * ()  const throw();
    T *  operator -> () const throw();
    
    // we just reference to the object refered by input <auto_reference>
    SIPSDK_AUTO_REF& operator = (SIPSDK_AUTO_REF&) throw();

private:
    T * p_;
};

template<class T>
SIPSDK_AUTO_REF<T>::SIPSDK_AUTO_REF(T * p) throw()
                  : p_(p)
{
    if(p_)
        p_->AddRef();
}

template<class T>
SIPSDK_AUTO_REF<T>::SIPSDK_AUTO_REF(SIPSDK_AUTO_REF & a) throw()
                  : p_(a.release())
{
}

template<class T>
SIPSDK_AUTO_REF<T>::~SIPSDK_AUTO_REF()
{
    if(p_)
        p_->ReleaseRef();
}

template<class T>
void SIPSDK_AUTO_REF<T>::reset(T * p) throw()
{
    if (p_ != p)
    {
        if(p_)
            p_->ReleaseRef();

        p_ = p;
        
        // Ricky added, 2010-1-27 13:44:05
        // We refer to a new T*
        if(p_)
            p_->AddRef();
    }
}

template<class T>
T * SIPSDK_AUTO_REF<T>::get() const  throw()
{
    return p_;
}

template<class T>
T * SIPSDK_AUTO_REF<T>::release()  throw()
{
    T * p = p_;
    p_ = NULL;

    return p;
}

template<class T>
T & SIPSDK_AUTO_REF<T>::operator * () const throw()
{
    return *p_;
}

template<class T>
T * SIPSDK_AUTO_REF<T>::operator -> () const throw()
{
    return p_;
}

template<class T>
SIPSDK_AUTO_REF<T>& SIPSDK_AUTO_REF<T>::operator = (SIPSDK_AUTO_REF<T> & a) throw()
{
    this->reset(a.get());
    return *this;
}

class CReferenceControl
{
public:
    CReferenceControl();
    virtual ~CReferenceControl();
    
    void add_reference();
    void release_reference();

protected:
    virtual void OnReferenceDestory();
    
    Mutex           mutex_;
    uint32          ref_cnt_;
};

template < class MutexType, typename T = uint32>
class CAtomicType 
{
public:
    CAtomicType(const T & val = 0)
            : count_num_(val),
              mutex_()
    {
    }
    
    CAtomicType & operator = (const CAtomicType & atom)
    {
        if(this == &atom)
            return *this;

        Lock lock(mutex_);
        count_num_ = atom.GetValue();
        
        return *this;
    }
    
    CAtomicType & operator = (const T & val)
    {
        Lock lock(mutex_);
        count_num_ = val;
        
        return *this;
    }
    
    T decr()
    {
        Lock lock(mutex_);
        return -- count_num_;
    }
    
    T incr()
    {
        Lock lock(mutex_);
        return ++ count_num_;
    }
    
    CAtomicType & operator -- ()
    {
        Lock lock(mutex_);
        -- count_num_;
        
        return *this;
    }
    
    CAtomicType & operator ++ ()
    {
        Lock lock(mutex_);
        ++ count_num_;
        
        return *this;
    }
    
    CAtomicType & operator += (const T & val)
    {
        Lock lock(mutex_);
        count_num_ += val;
     
        return *this;
    }       
    
    CAtomicType & operator -= (const T & val)
    {
        Lock lock(mutex_);
        count_num_ -= val;
        
        return *this;
    }
    
    operator T ()
    {
        Lock lock(mutex_);
        return count_num_;
    }

    T GetValue()
    {
        Lock lock(mutex_);
        return count_num_;
    }
    
    void SetValue(const T & val)
    {
        Lock lock(mutex_);
        this->count_num_ = val;
    }
    
    bool operator == (const T & val)
    {
        Lock lock(mutex_);
        return count_num_ == val;
    }

private:
    CAtomicType(const CAtomicType & );

    CAtomicType<MutexType, T>   operator -- (int); // X ++
    CAtomicType<MutexType, T>   operator ++ (int); // X --

    T           count_num_;
    MutexType   mutex_;
};

typedef CAtomicType<Mutex, uint32>  AtomicUInteger;
typedef CAtomicType<Mutex, bool>    AtomicBool;

// A macro to disallow the evil copy constructor and operator= functions
// This should be used in the private: declarations for a class
#define DISALLOW_EVIL_CONSTRUCTORS(TypeName) \
    TypeName(const TypeName&);  \
    TypeName& operator=(const TypeName&)

// A macro to disallow all the implicit constructors, namely the
// default constructor, copy constructor and operator= functions.
//
// This should be used in the private: declarations for a class
// that wants to prevent anyone from instantiating it. This is
// especially useful for classes containing only static methods.
#define DISALLOW_IMPLICIT_CONSTRUCTORS(TypeName) \
    TypeName(); \
    DISALLOW_EVIL_CONSTRUCTORS(TypeName)

typedef enum Ordix
{    HEX     = 0,
     DECIMAL = 1,
     S_FLOAT = 2,
     L_FLOAT
};

//////////////////////////////////////////////////////////////////////////////////////
// Example:  cout << TIME_PRT<>(-1, "%a, %d %b %Y %H:%M:%S %Z") << endl;
//
// Refer to strftime() for [pattern] explaination
// time == -1 means current time
template < bool GMT = false >
class TIME_PRT
{
public:
    explicit TIME_PRT(time_t time, LPCSTR pattern)
        : str_len_(0)
    {
        *buf_ = '\0';
        
        time_t t = time;
        if(t == -1)
            ::time(&t);
                
        struct tm tmBuf;
        if(GMT)
            ::gmtime_r(&t, &tmBuf);
        else
            ::localtime_r(&t, &tmBuf);

        if( 0 == (str_len_ = ::strftime(buf_, sizeof(buf_), pattern, &tmBuf)) )
            *buf_ = '\0';
    }
    
    ~TIME_PRT() {}
    
    operator LPCSTR() const { return this->buf_;        }
    int      length() const { return this->str_len_;    }

private:
    char   buf_[32];
    int    str_len_;
};

class SIPSDKTextFormator
{
public :
    SIPSDKTextFormator(char* lpszBuf, uint32 dwBufSize);
    virtual ~SIPSDKTextFormator();

public :
    void reset();
    operator char*();

    SIPSDKTextFormator& operator << (bool);
    SIPSDKTextFormator& operator << (char);
    SIPSDKTextFormator& operator << (BYTE);
    SIPSDKTextFormator& operator << (short);
    SIPSDKTextFormator& operator << (unsigned short);
    SIPSDKTextFormator& operator << (int);
    SIPSDKTextFormator& operator << (unsigned int);
    SIPSDKTextFormator& operator << (long);
    SIPSDKTextFormator& operator << (unsigned long);
    SIPSDKTextFormator& operator << (int64);
    SIPSDKTextFormator& operator << (uint64);
    SIPSDKTextFormator& operator << (float);
    SIPSDKTextFormator& operator << (double);
    SIPSDKTextFormator& operator << (LPSTR);
    SIPSDKTextFormator& operator << (LPCSTR);
    SIPSDKTextFormator& operator << (const string& );
    SIPSDKTextFormator& operator << (LPVOID);
    SIPSDKTextFormator& operator << (Ordix);
    
private :
    DISALLOW_EVIL_CONSTRUCTORS(SIPSDKTextFormator);

    void set_hex_flag(bool bValue);
    bool get_hex_flag();
    void advance(const char* lpsz);
    
    char*       m_pBuf;
    uint32      m_dwSize;
    uint32      m_dwPos;
    bool        m_bHex;
    bool        m_bLongFloat;
};

extern "C"
{
    uint32 Log2(uint32 num);
    
    /* 
     * this function is to parse a key-value buffer to key-value pair
     * 
     * Suppose p = "expires = 3000;", and len = 15;
     * The result would be:
     * key = "expires"; value = "3000";
     */
    bool ParseKV(LPCSTR p, int len, StrRef key, StrRef value);
    
    /* 
     * this function is to parse a key-value buffer to key-value pair
     * 
     * Suppose buf = "expires = 3000      ;xxxx; q = 3.0;;;; ;xxx; 333=222;444=;4xx44="
     * The result would be:
     * "expires" -- "3000"
     * "q"       -- "3"
     */
    void ParseSipAttr(CStrRef buf, map<string, string> & result);
    
    
    /*
     * trim a string left & right space
     * buf = "  expires = 3000; " return "expires = 3000;"
     */
    string TrimString(CStrRef buf);
    
    // case-insensitive compare between l & r, "hello" == "HellO".
    bool is_equals(CStrRef l, CStrRef r);
    
    // to say if code is 2xx
    bool is_2xx(int code);
}

#endif
