// This is to tell the basic implement of string in CGI STL, and discuss about
// COW of string. COW of string will bring multi-thread cost, so deep copy
// is more popular.

// Reference:
// 1. http://blog.csdn.net/pizi0475/article/details/5288432
// 2. http://www.cnblogs.com/lfsblack/archive/2012/11/10/2764334.html
// 3. <STL源码剖析>

// This is used for basic_string.
struct _Rep_base
{
    size_type  _M_length;   // Real length of string, not include NULL at the end.
    size_type  _M_capaticty;    // > len
    int        _M_refCount; // For multi-object point to the string object.
};

// basic_string is defined in basic_string.h, here just simulation it.
template <typename T, typename TraitsT, typename Alloc>
class basic_string {
    struct _Rep_base
};

// This is string implement in CGI STL, in stl_string_fwd.h .
template <class _CharT,
    // Refer to <3>, traits is to present iterator's value type. This is to
    // supply basic_string<char>::value_type, which is _CharT.
    class _Traits = char_traits<_CharT>,
    class _Alloc = _STL_DEFAULT_ALLOCATOR(_CharT)>
class basic_string; // basic_string has defined in other file, here declare
                    // it in this file.

typedef basic_string<char> string;
