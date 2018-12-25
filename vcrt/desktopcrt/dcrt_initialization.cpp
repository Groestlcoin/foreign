//
// initialization.cpp
//
//      Copyright (c) Microsoft Corporation.  All rights reserved.
//
// This file defines the main initialization and uninitialization routines for
// the DesktopCRT, shared by both the static and dynamic DesktopCRT libraries.
// In the dynamic DesktopCRT library, these are called by DllMain.  In the static
// DesktopCRT library, these are called by the initialization code.
//
#include <corecrt_internal.h>
#include <appmodel.h>



#ifdef CRTDLL

    static bool __cdecl initialize_c() throw()
    {
        // Do C initialization:
        if (_initterm_e(__xi_a, __xi_z) != 0)
            return false;

        // Do C++ initialization:
        _initterm(__xc_a, __xc_z);
        return true;
    }

    static bool __cdecl uninitialize_c() throw()
    {
        // Do pre-termination:
        _initterm(__xp_a, __xp_z);

        // Do termination:
        _initterm(__xt_a, __xt_z);
        return true;
    }

#else

    static bool __cdecl initialize_c() throw()
    {
        return true;
    }

    static bool __cdecl uninitialize_c() throw()
    {
        return true;
    }

#endif

extern "C" bool __cdecl __dcrt_initialize()
{
    #ifdef _CRT_GLOBAL_STATE_ISOLATION
    // Configure CRT's per-thread global state mode data
    if (!__crt_state_management::initialize_global_state_isolation())
        return false;
    #endif

    #if defined CRTDLL && !defined _M_CRT_UNSUPPORTED
    __isa_available_init();
    #endif
    
    if (!__dcrt_initialize_ptd())
        return false;

    __acrt_initialize_multibyte();

    // In the static CRT, the environment is initialized in the static startup
    // code.  We do this to prevent the narrow environment from being initialized
    // earlier than it needs to be.
    #ifdef CRTDLL
    if (__dcrt_initialize_narrow_environment_nolock() < 0)
        return false;
    
    __dcrt_initial_narrow_environment = __dcrt_get_or_create_narrow_environment_nolock();
    if (!__dcrt_initial_narrow_environment)
        return false;
    #endif

    __acrt_post_initialize(GetModuleHandleW(_CRT_WIDE(_LIBRARYLOADER_MODULE_NAME)));

    // CRT_REFACTOR TODO We need to be sure that when the DesktopCRT terminates,
    // we notify the AppCRT to clean up any references that it might have to
    // DesktopCRT functionality.
    if (!initialize_c())
    {
        return false;
    }

    return true;
}

extern "C" bool __cdecl __dcrt_uninitialize(bool const terminating)
{
    UNREFERENCED_PARAMETER(terminating);

    uninitialize_c();

    // If the process is terminating, there's no point in cleaning up, except
    // in debug builds.
    #ifndef _DEBUG
    if (!terminating)
    #endif
    {
        __dcrt_uninitialize_environments_nolock();

        __dcrt_uninitialize_ptd();
        
    #ifdef _CRT_GLOBAL_STATE_ISOLATION
        __crt_state_management::uninitialize_global_state_isolation(terminating);
    #endif
    }

    return true;
}

extern "C" bool __cdecl __dcrt_uninitialize_critical()
{
    __dcrt_uninitialize_ptd();
    
    #ifdef _CRT_GLOBAL_STATE_ISOLATION
    __crt_state_management::uninitialize_global_state_isolation(true);
    #endif

    return true;
}

extern "C" bool __cdecl __dcrt_thread_attach()
{
    return true;
}

extern "C" bool __cdecl __dcrt_thread_detach()
{
    return true;
}
