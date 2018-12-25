//
// fp8.cpp
//
//      Copyright (c) Microsoft Corporation. All rights reserved.
//
// Implementation of _setdefaultprecision that sets the precision to 53 bits (8-
// byte double).  This behavior can be overridden by linking with the fp10 link
// option.
//
#undef MRTDLL

#include <corecrt_internal.h>
#include <float.h>

// Routine to set default FP precision to 53 bits.
extern "C" void __CLRCALL_OR_CDECL _setdefaultprecision()
{
    _ERRCHECK(_controlfp_s(nullptr, _PC_53, _MCW_PC));
}
