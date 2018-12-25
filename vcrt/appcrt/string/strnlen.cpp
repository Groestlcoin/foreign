//
// strnlen.cpp
//
//      Copyright (c) Microsoft Corporation.  All rights reserved.
//
// Defines strnlen(), which returns the length of a null-terminated string, not
// including the null terminator itself, up to the specified maximum size.
//
#include <string.h>



extern "C" size_t __cdecl strnlen(
	char const* const string,
	size_t      const maximum_size
	)
{
    // Note that we do not check if the string is null because we do not return
    // an error code.
    size_t n = 0;
    for (
    	char const* it = string;
    	n < maximum_size && *it;
    	++n, ++it
    ) { }

    return n;
}
