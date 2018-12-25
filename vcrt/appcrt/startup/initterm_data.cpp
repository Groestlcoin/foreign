//
// initterm_data.cpp
//
//      Copyright (c) Microsoft Corporation. All rights reserved.
//
// Data used in C and C++ initialization and termination.  Because this object
// is linked into every module that uses the CRT, we also use this module to
// pass flags to the linker to link with libraries upon which the CRT depends.
//
#include <vcruntime_internal.h>	//!!!P
//#include <corecrt_internal.h>



extern "C" {



_CRTALLOC(".CRT$XIA") _PIFV __xi_a[] = { nullptr }; // C initializers (first)
_CRTALLOC(".CRT$XIZ") _PIFV __xi_z[] = { nullptr }; // C initializers (last)
_CRTALLOC(".CRT$XCA") _PVFV __xc_a[] = { nullptr }; // C++ initializers (first)
_CRTALLOC(".CRT$XCZ") _PVFV __xc_z[] = { nullptr }; // C++ initializers (last)
_CRTALLOC(".CRT$XPA") _PVFV __xp_a[] = { nullptr }; // C pre-terminators (first)
_CRTALLOC(".CRT$XPZ") _PVFV __xp_z[] = { nullptr }; // C pre-terminators (last)
_CRTALLOC(".CRT$XTA") _PVFV __xt_a[] = { nullptr }; // C terminators (first)
_CRTALLOC(".CRT$XTZ") _PVFV __xt_z[] = { nullptr }; // C terminators (last)

#pragma comment(linker, "/merge:.CRT=.rdata")



// CRT_REFACTOR TODO There are two TODOs here:
//  1. Linking with kernel32.lib is the right thing to do for the Desktop flavor
//     of the CRT.  It is not the right thing to do for CoreSys, KernelX, etc.
//     At some point, we'll need to revisit this.
//
//  2. The /disallowlib options will need to be updated to disallow incompatible
//     combinations of the new refactored CRT libraries (appcrt.lib, etc.).  We
//     may decide to split this logic up across the various parts of the new CRT



// Link with kernel32.lib to resolve Platform APIs required by the static code
#pragma comment(linker, "/defaultlib:kernel32.lib")



// Disallow mixing of incompatible CRT libraries (e.g. /MT with msvcrt.lib)
#ifdef CRTDLL

    #pragma comment(linker, "/disallowlib:libcmt.lib")
    #pragma comment(linker, "/disallowlib:libcmtd.lib")

    #ifdef _DEBUG
        #pragma comment(linker, "/disallowlib:msvcrt.lib")
    #else
        #pragma comment(linker, "/disallowlib:msvcrtd.lib")
    #endif

#else

    #pragma comment(linker, "/disallowlib:msvcrt.lib")
    #pragma comment(linker, "/disallowlib:msvcrtd.lib")

    #ifdef _DEBUG
        #pragma comment(linker, "/disallowlib:libcmt.lib")
    #else
        #pragma comment(linker, "/disallowlib:libcmtd.lib")
    #endif

#endif



} // extern "C"
