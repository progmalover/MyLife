#include "SipUtil.h"

///////////////////////////////////////////////////////
// Implementation of CReferenceControl
//
CReferenceControl::CReferenceControl()
                  : mutex_(),
                    ref_cnt_(0)
{
}

CReferenceControl::~CReferenceControl()
{
}

void CReferenceControl::add_reference()
{
    Lock lock(mutex_);
    ++ ref_cnt_;
}

void CReferenceControl::release_reference()
{
    uint32 ref_cnt_temp = 0;
    {
        Lock lock(mutex_);
        if(ref_cnt_ > 0)
        {
            -- ref_cnt_;
            ref_cnt_temp = ref_cnt_;
        }
        else
        {
            // set an invalid value to ref_cnt_temp
            ref_cnt_temp = (uint32)-1;
        }
    }
    
    if (0 == ref_cnt_temp)
        this->OnReferenceDestory();
}

void CReferenceControl::OnReferenceDestory()
{
    delete this;
}


/////////////////////////////////////////////////////////////////////////////
// Inner class SIPSDKTextFormator
//
SIPSDKTextFormator::SIPSDKTextFormator(LPSTR lpszBuf, uint32 dwBufSize)
    : m_pBuf(lpszBuf),
      m_dwSize(dwBufSize),
      m_dwPos(0),
      m_bHex(false),
      m_bLongFloat(true)
{
    reset();
}

SIPSDKTextFormator::~SIPSDKTextFormator()
{
}

void SIPSDKTextFormator::reset()
{
    m_dwPos      = 0;
    m_bHex       = false;
    m_bLongFloat = true;
    
    memset(m_pBuf, 0, m_dwSize);
}

SIPSDKTextFormator& SIPSDKTextFormator::operator << (bool b)
{
    char achBuf[6];
    sprintf(achBuf, "%s", b ? "true" : "false");
    advance(achBuf);
    
    return *this;
}

SIPSDKTextFormator& SIPSDKTextFormator::operator << (char ch)
{
    char achBuf[2];
    sprintf(achBuf, "%c", ch);
    advance(achBuf);
    
    return *this;
}

SIPSDKTextFormator& SIPSDKTextFormator::operator << (BYTE ch)
{
    return *this << (int)ch;
}

SIPSDKTextFormator& SIPSDKTextFormator::operator << (short s)
{
    return *this << (int)s;
}

SIPSDKTextFormator& SIPSDKTextFormator::operator << (unsigned short s)
{
    return *this << (int)s;
}

SIPSDKTextFormator& SIPSDKTextFormator::operator << (int i)
{
    char achBuf[20];
    
    if(!get_hex_flag())
        sprintf(achBuf, "%d", i);
    else
        sprintf(achBuf, "%x", i);
    
    advance(achBuf);
    set_hex_flag(false);
    
    return *this;
}

SIPSDKTextFormator& SIPSDKTextFormator::operator << (unsigned int i)
{
    char achBuf[20];
    
    if(!get_hex_flag())
        sprintf(achBuf, "%u", i);
    else
        sprintf(achBuf, "%x", i);
    
    advance(achBuf);
    set_hex_flag(false);
    
    return *this;
}

SIPSDKTextFormator& SIPSDKTextFormator::operator << (long l)
{
    char achBuf[20];
    
    if(!get_hex_flag())
        sprintf(achBuf, "%ld", l);
    else
        sprintf(achBuf, "%lx", l);
    
    advance(achBuf);
    set_hex_flag(false);
    return *this;
}

SIPSDKTextFormator& SIPSDKTextFormator::operator << (unsigned long l)
{
    char achBuf[20];
    
    if(!get_hex_flag())
        sprintf(achBuf, "%lu", l);
    else
        sprintf(achBuf, "%lx", l);
    
    advance(achBuf);
    set_hex_flag(false);
    
    return *this;
}

SIPSDKTextFormator& SIPSDKTextFormator::operator << (int64 ll)
{
    char achBuf[40];
    
    if(!get_hex_flag())
        sprintf(achBuf, "%lld", ll);
    else
        sprintf(achBuf, "%llx", ll);
    
    advance(achBuf);
    set_hex_flag(false);
    return *this;
}

SIPSDKTextFormator& SIPSDKTextFormator::operator << (uint64 ull)
{
    char achBuf[40];
    
    if(!get_hex_flag())
        sprintf(achBuf, "%llu", ull);
    else
        sprintf(achBuf, "%llx", ull);
    
    advance(achBuf);
    set_hex_flag(false);
    return *this;
}

SIPSDKTextFormator& SIPSDKTextFormator::operator << (float f)
{
    char achBuf[40];
    sprintf(achBuf, "%f", f);
    
    advance(achBuf);
    return *this;
}

SIPSDKTextFormator& SIPSDKTextFormator::operator << (double d)
{
    char achBuf[40];
    
    if(m_bLongFloat)
        sprintf(achBuf, "%f", d);
    else
        sprintf(achBuf, "%.3f", d);
    
    advance(achBuf);
    return *this;
}

SIPSDKTextFormator& SIPSDKTextFormator::operator << (LPSTR lpsz)
{
    advance(lpsz);
    return *this;
}

SIPSDKTextFormator& SIPSDKTextFormator::operator << (LPCSTR lpsz)
{
    advance(lpsz);
    return *this;
}

SIPSDKTextFormator& SIPSDKTextFormator::operator << (const string & str)
{
    advance(str.c_str());
    return *this;
}

SIPSDKTextFormator& SIPSDKTextFormator::operator << (LPVOID lpv)
{
    *this << "0x" << HEX << (ULONG)lpv;
    return *this;
}

SIPSDKTextFormator& SIPSDKTextFormator::operator << (Ordix ordix)
{
    switch(ordix)
    {
    case HEX :
        set_hex_flag(true);
        break;
        
    case DECIMAL :
        set_hex_flag(false);
        break;

    case S_FLOAT:
        m_bLongFloat = false;
        break;
    
    case L_FLOAT:
        m_bLongFloat = true;
        
    default :
        break;
    }
    return *this;
}

SIPSDKTextFormator::operator char*()
{
    return m_pBuf;
}

void SIPSDKTextFormator::set_hex_flag(bool bValue)
{
    m_bHex = bValue;
}

bool SIPSDKTextFormator::get_hex_flag()
{
    return m_bHex;
}

void SIPSDKTextFormator::advance(LPCSTR lpsz)
{
    if(lpsz)
    {
        uint32 nLength = (uint32)strlen(lpsz);
        if(nLength > m_dwSize - m_dwPos - 64)
            nLength = m_dwSize - m_dwPos - 64;
        
        if(nLength > 0)
        {
            memcpy(m_pBuf + m_dwPos*sizeof(char), lpsz, 
                nLength*sizeof(char));
            m_dwPos += nLength;
        }
    }
}

uint32 Log2(uint32 num)
{
    uint32 cnt = 0;
    
    while(num = num >> 1)
        cnt ++;

    return cnt;
}

/* 
 * this function is to parse a key-value buffer to key-value pair
 * 
 * Suppose p = "expires = 3000;", and len = 15;
 * The result would be:
 * key = "expires"; value = "3000";
 */
bool ParseKV(LPCSTR p, int len, StrRef key, StrRef value )
{
    if(!p || isalnum(*p) == 0 || len <= 0)
        return false;
    
    LPCSTR end = p + len;
    LPCSTR p1  = p;
    while('=' != *p1 && end > p1) ++ p1;
    
    // key right trim
    {
        -- p1;
        while(isspace(*p1)) -- p1;
        ++ p1;
    }
    
    // validate key
    if(p1 <= p)
        return false;
        
    key.assign(p, p1);
    
    while(isspace(*p1)) ++ p1;
    if(*p1 != '=')
        return false;

    // value left trim
    {
        ++ p1;
        while(isspace(*p1)) ++ p1;
        p = p1;
    }
    
    // value right trim
    {
        while(*p1 != '\0' && end > p1) ++ p1;
        
        -- p1;
        while((isspace(*p1) || *p1 == ';') && p1 >= p) -- p1;
        ++ p1;
    }
    
    // validate value
    if(p1 <= p)
        return false;
    
    value.assign(p, p1);
    return true;        
}

/* 
 * this function is to parse a key-value buffer to key-value pair
 * 
 * Suppose buf = "expires = 3000      ;xxxx; q = 3.0;;;; ;xxx; 333=222;444=;4xx44="
 * The result would be:
 * "expires" -- "3000"
 * "q"       -- "3"
 */

void ParseSipAttr(CStrRef buf, map<string, string> & result)
{
    //TODO: implementation
    int len          = buf.length();
    LPCSTR    p      = buf.c_str();
    LPCSTR    end    = p + len;
    
    do
    {
        //trim left
        while('\0' != *p && isspace(*p)) ++ p;

        LPCSTR q = strchr(p, ';');
        if(!q) q = end;
        
        string k, v;
        if(ParseKV(p, q - p, k, v))
            result[k] = v;
        
        p = ++ q;
    } while(p < end);
}

 /*
  * trim a string left & right space
  * buf = "  expires = 3000; " return "expires = 3000;"
  */
string TrimString(CStrRef buf)
{
    LPCSTR p = buf.c_str();
    LPCSTR q = p + buf.length();
    
    for(; isspace(*p) && p <= q;) ++ p;

    for(; (*q == '\0' || isspace(*q)) && p <= q;) -- q;
    ++ q;
    
    string res;
    if(p < q) 
        res.assign(p, q);
    
    return res;
}

bool is_equals(CStrRef l, CStrRef r)
{
    if (l.size() != r.size())
        return false;

    string::const_iterator c1  = l.begin(), c2 = r.begin();
    for ( ; c1 != l.end(); ++ c1, ++ c2 )
    {
        if (tolower(*c1) != tolower(*c2))
            return false;
    }

    return true;
}

bool is_2xx(int code)
{
    return (code >= 200 && code < 300);
}
