//
// wcsnlen.cpp
//
//      Copyright (c) Microsoft Corporation.  All rights reserved.
//
// Defines wcsnlen(), which computes the length of a wide character string,
// examining at most 'max_size' characters.
//
#include <string.h>



extern "C" size_t __cdecl wcsnlen(
	wchar_t const* const string,
	size_t         const max_size
	)
{
    size_t n = 0;
    for (
    	wchar_t const* p = string;
    	*p && n != max_size;
    	++p, ++n
    ) { }

    return n;
}
