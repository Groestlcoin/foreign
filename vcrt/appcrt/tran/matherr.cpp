/***
*matherr.cpp - floating point exception handling
*
*       Copyright (c) Microsoft Corporation. All rights reserved.
*
*Purpose:
*
*******************************************************************************/

#include <corecrt_internal.h>
#include <math.h>
#include <stddef.h>



typedef int (__cdecl* _HANDLE_MATH_ERROR)(_exception*);

static __crt_state_management::dual_state_global<_HANDLE_MATH_ERROR> user_matherr;



extern "C" _ACRTIMP void __cdecl __setusermatherr(_HANDLE_MATH_ERROR const pf)
{
    user_matherr.value() = __crt_fast_encode_pointer(pf);
}

extern "C" void __cdecl __acrt_initialize_user_matherr(void* const encoded_null)
{
    user_matherr.initialize(static_cast<_HANDLE_MATH_ERROR>(encoded_null));
}

extern "C" bool __cdecl __acrt_has_user_matherr()
{
    return __crt_fast_decode_pointer(user_matherr.value()) != nullptr;
}

extern "C" int __cdecl __acrt_invoke_user_matherr(_exception* const math_exception)
{
    // If user has supplied a _matherr implementation (__setusermatherr has
    // been called), pass control to it and let it handle the error.
    _HANDLE_MATH_ERROR const local_matherr = __crt_fast_decode_pointer(user_matherr.value());
    if (!local_matherr)
        return 0;

    return local_matherr(math_exception);
}
