//
// wcslen.cpp
//
//      Copyright (c) Microsoft Corporation.  All rights reserved.
//
// Defines wcslen(), which computes the length of a wide character string.
//
#include <string.h>



#if defined(_M_X64) || defined(_M_IX86) || defined(_M_ARM)
    #pragma warning(disable:4163)
    #pragma function(wcslen)
#endif



extern "C" size_t __cdecl wcslen(wchar_t const* const string)
{
    wchar_t const* string_it = string;
    while (*string_it++) { }

    return static_cast<size_t>(string_it - string - 1);
}
