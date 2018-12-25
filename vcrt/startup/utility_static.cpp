//
// utility_static.cpp
//
//      Copyright (c) Microsoft Corporation. All rights reserved.
//
// CRT startup and termination functionality specific to use of the static CRT.
// The functions defined here are either (a) not used when the CRT DLLs are used
// or (b) have different implementations when the CRT DLLs are used.
//
#ifdef CRTDLL
    #error This file should only be built into the static CRT
#endif

#include <vcstartup_internal.h>
#include <isa_availability.h>
#include <process.h>



//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//
// Startup Synchronization
//
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
extern "C" bool __cdecl __scrt_acquire_startup_lock()
{
    return false;
}

extern "C" void __cdecl __scrt_release_startup_lock(bool const is_nested)
{
    UNREFERENCED_PARAMETER(is_nested);
}



//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//
// CRT Initialization
//
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
static bool is_initialized_as_dll;

extern "C" bool __cdecl __scrt_initialize_crt(__scrt_module_type const module_type)
{
    if (module_type == __scrt_module_type::dll)
    {
        is_initialized_as_dll = true;
    }

    __isa_available_init();
    
    // Notify the CRT components of the process attach, bottom-to-top:
    if (!__vcrt_initialize())
    {
        return false;
    }

    if (!__acrt_initialize())
    {
        __vcrt_uninitialize(FALSE);
        return false;
    }

    return true;
}

extern "C" bool __cdecl __scrt_uninitialize_crt(bool const is_terminating, bool const from_exit)
{
    // If the CRT is statically linked into a DLL and that DLL calls exit(), we
    // do not uninitialize the statically-linked CRT.  Instead, we wait for the
    // DLL_PROCESS_DETACH notification that we will receive after ExitProcess()
    // is called by exit().
    if (is_initialized_as_dll && from_exit)
    {
        return true;
    }

    // Notify the CRT components of the process detach, top-to-bottom:
    __acrt_uninitialize(is_terminating);
    __vcrt_uninitialize(is_terminating);

    return true;
}



//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//
// On-Exit Table
//
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
extern "C" _onexit_t __cdecl _onexit(_onexit_t const function)
{
    return _crt_atexit(reinterpret_cast<_PVFV>(function)) == 0
        ? function
        : nullptr;
}

extern "C" int __cdecl atexit(_PVFV const function)
{
    return _crt_atexit(function);
}

extern "C" int __cdecl at_quick_exit(_PVFV const function)
{
    return _crt_at_quick_exit(function);
}

extern "C" bool __cdecl __scrt_initialize_onexit_tables(__scrt_module_type)
{
    // No-op.  When the static CRT libraries are used, the onexit table is shared
    // by the CRT and the module into which the CRT is linked.
    return true;
}



//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//
// DLL Functionality
//
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
extern "C" int __cdecl __scrt_dllmain_exception_filter(
    HINSTANCE           const instance,
    DWORD               const reason,
    LPVOID              const reserved,
    __scrt_dllmain_type const crt_dllmain,
    unsigned long       const exception_code_,
    PEXCEPTION_POINTERS const exception_info_
    )
{
    if (reason == DLL_PROCESS_ATTACH)
    {
        // The client DllMain routine failed with an unhandled exception, so the
        // in-module static CRT needs to be cleaned up.  We do this by calling
        // the CRT DllMain equivalent with DLL_PROCESS_DETACH.  Note that we do
        // not call the client DllMain again because the DLL has not completed
        // loading, and we do not know what its state is due to the unhandled
        // exception.
        crt_dllmain(instance, DLL_PROCESS_DETACH, reserved);
    }

    return _seh_filter_dll(exception_code_, exception_info_);
}

extern "C" bool __cdecl __scrt_dllmain_before_initialize_c()
{
    return true;
}

extern "C" bool __cdecl __scrt_dllmain_after_initialize_c()
{
    _initialize_narrow_environment();
    return true;
}

extern "C" void __cdecl __scrt_dllmain_uninitialize_c()
{
    if (!_is_c_termination_complete())
        _cexit();
}

extern "C" void __cdecl __scrt_dllmain_uninitialize_critical()
{
    // The critical uninitialization code should be unnecessary.  When an
    // exception escapes from DllMain during DLL_PROCESS_DETACH, the OS
    // should terminate the process.  Unfortunately, through Windows 7, the
    // exception is silently swallowed.
    //
    // Therefore, we uninitialize the PTD during phase two unwind.  This
    // ensures that our FLS callback is unregistered (we must unregister it
    // so that it is not left pointing to code in the CRT module, which is
    // about to be unloaded).
    //
    // Note that the CRT is in one of three possible states here:
    // (1) Uninitialization completed successfully, so the PTD has already
    //     been uninitialized.  In this case, the call here has no effect.
    // (2) Uninitialization failed, but after the PTD was uninitialized.
    //     Again, in this case, the call here has no effect.
    // (3) Uninitialization failed, before the PTD was uninitialized.  In
    //     this case, the call here will uninitialize the PTD and remove
    //     the FLS callback.
    __acrt_uninitialize_critical(false);
    __vcrt_uninitialize_critical();
}

extern "C" bool __cdecl __scrt_dllmain_crt_thread_attach()
{
    // Notify the CRT components of the thread attach, bottom-to-top:
    if (!__vcrt_thread_attach())
    {
        return false;
    }

    if (!__acrt_thread_attach())
    {
        __vcrt_thread_detach();
        return false;
    }

    return true;
}

extern "C" bool __cdecl __scrt_dllmain_crt_thread_detach()
{
    // Notify the CRT components of the thread detach, top-to-bottom:
    __acrt_thread_detach();
    __vcrt_thread_detach();
    return true;
}
