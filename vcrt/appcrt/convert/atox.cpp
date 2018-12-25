//
// atox.cpp
//
//      Copyright (c) Microsoft Corporation. All rights reserved.
//
// The "simple" string conversion functions:  atoi, atol, atoll, and their wide
// string functions.  They behave exactly like the str* and wcs* functions that
// they call.
//
#include <corecrt_internal.h>
#include <ctype.h>
#include <locale.h>
#include <stdlib.h>



//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//
// Narrow Strings => Various Integers (Simple Functions, wtox)
//
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
extern "C" int __cdecl atoi(char const* const string)
{
    return atol(string);
}

extern "C" int __cdecl _atoi_l(char const* const string, _locale_t const locale)
{
    return _atol_l(string, locale);
}

extern "C" long __cdecl atol(char const* const string)
{
    return strtol(string, nullptr, 10);
}

extern "C" long __cdecl _atol_l(char const* const string, _locale_t const locale)
{
    return _strtol_l(string, nullptr, 10, locale);
}

extern "C" long long __cdecl atoll(char const* const string)
{
    return _strtoi64(string, nullptr, 10);
}

extern "C" long long __cdecl _atoll_l(char const* const string, _locale_t const locale)
{
    return _strtoi64_l(string, nullptr, 10, locale);
}

extern "C" __int64 __cdecl _atoi64(char const* const string)
{
    return atoll(string);
}

extern "C" __int64 __cdecl _atoi64_l(char const* const string, _locale_t const locale)
{
    return _atoll_l(string, locale);
}



//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//
// Wide Strings => Various Integers (Simple Functions, wtox)
//
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
extern "C" int __cdecl _wtoi(wchar_t const* const string)
{
    return _wtol(string);
}

extern "C" int __cdecl _wtoi_l(wchar_t const* const string, _locale_t const locale)
{
    return _wtol_l(string, locale);
}

extern "C" long __cdecl _wtol(wchar_t const* const string)
{
    return wcstol(string, nullptr, 10);
}

extern "C" long __cdecl _wtol_l(wchar_t const* const string, _locale_t const locale)
{
    return _wcstol_l(string, nullptr, 10, locale);
}

extern "C" long long __cdecl _wtoll(wchar_t const* const string)
{
    return _wcstoi64(string, nullptr, 10);
}

extern "C" long long __cdecl _wtoll_l(wchar_t const* const string, _locale_t const locale)
{
    return _wcstoi64_l(string, nullptr, 10, locale);
}

extern "C" __int64 __cdecl _wtoi64(wchar_t const* const string)
{
    return _wtoll(string);
}

extern "C" __int64 __cdecl _wtoi64_l(wchar_t const* const string, _locale_t const locale)
{
    return _wtoll_l(string, locale);
}
