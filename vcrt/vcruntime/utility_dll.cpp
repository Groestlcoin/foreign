//
// utility_dll.cpp
//
//      Copyright (c) Microsoft Corporation. All rights reserved.
//
// CRT startup and termination functionality specific to use of the CRT DLL.
// The functions defined here are either (a) not used when the static CRT is
// used or (b) have different implementations when the static CRT is used.
//
#ifndef CRTDLL
    #error This file should only be built into the CRT DLLs
#endif

#include <vcstartup_internal.h>



//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//
// Startup Synchronization
//
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
extern "C" bool __cdecl __scrt_acquire_startup_lock()
{
    void* const this_fiber = reinterpret_cast<PNT_TIB>(NtCurrentTeb())->StackBase;

    while (void* const owning_fiber = _InterlockedCompareExchangePointer(&__scrt_native_startup_lock, this_fiber, nullptr))
    {
        if (this_fiber == owning_fiber)
            return true;
    }

    return false;
}

extern "C" void __cdecl __scrt_release_startup_lock(bool const is_nested)
{
    if (is_nested)
        return;

    _InterlockedExchangePointer(&__scrt_native_startup_lock, nullptr);
}



//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//
// CRT Initialization
//
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
extern "C" bool __cdecl __scrt_initialize_crt(__scrt_module_type)
{
    return true; // No action is required
}

extern "C" bool __cdecl __scrt_uninitialize_crt(bool const is_terminating, bool const from_exit)
{
    UNREFERENCED_PARAMETER(is_terminating);
    UNREFERENCED_PARAMETER(from_exit);

    return true; // No action is required
}



//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//
// On-Exit Table
//
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// When a module uses the CRT DLLs, the behavior of _onexit() and atexit() are
// different depending on whether the module is an EXE or a DLL.  If it is an
// EXE, then calls to _onexit() and atexit() are passed to the CRT DLL and
// registered with its on-exit function table.  This way, functions registered
// by the EXE are called whenever the exit() codepath is entered.
//
// If the module is a DLL, it has its own table of registered functions.  This
// table is executed when the DLL is unloaded (during DLL_PROCESS_DETACH).
//
// The functions here are module-local definitions of _onexit() and atexit().
// When this object is linked into an EXE, they simply call the _onexit()
// function exported by the CRT DLL.  When this object is linked into a DLL,
// they use the module-local on-exit table defined here.
//
// None of this is required when the static CRT is used.  With the static CRT,
// everything is module-local.
//
// The same applies for the at_quick_exit table.
static _onexit_table_t module_local_atexit_table{};
static _onexit_table_t module_local_at_quick_exit_table{};



extern "C" _onexit_t __cdecl _onexit(_onexit_t const function)
{
    _PVFV* const onexit_first = __crt_fast_decode_pointer(module_local_atexit_table._first);

    // If the onexit table is initialized with the sentinel pointer -1, then
    // this is an exe module, and we just register the function with the table
    // in the CRT DLL:
    if (onexit_first == reinterpret_cast<_PVFV*>(-1))
    {
        return _crt_atexit(reinterpret_cast<_PVFV>(function)) == 0
            ? function
            : nullptr;
    }
    // Otherwise, this module has its own onexit table, so it is a DLL; we need
    // to register the function with the module-local onexit table:
    else
    {
        return _register_onexit_function(&module_local_atexit_table, function) == 0
            ? function
            : nullptr;
    }
}

extern "C" int __cdecl atexit(_PVFV const function)
{
    return _onexit(reinterpret_cast<_onexit_t>(function)) != nullptr
        ? 0
        : -1;
}

extern "C" int __cdecl at_quick_exit(_PVFV const function)
{
    _PVFV* const onexit_first = __crt_fast_decode_pointer(module_local_at_quick_exit_table._first);
    
    // If the onexit table is initialized with the sentinel pointer -1, then
    // this is an exe module, and we just register the function with the table
    // in the CRT DLL:
    if (onexit_first == reinterpret_cast<_PVFV*>(-1))
    {
        return _crt_at_quick_exit(function);
    }
    // Otherwise, this module has its own onexit table, so it is a DLL; we need
    // to register the function with the module-local onexit table:
    else
    {
        _onexit_t const onexit_function = reinterpret_cast<_onexit_t>(function);
        return _register_onexit_function(&module_local_at_quick_exit_table, onexit_function);
    }
}

extern "C" bool __cdecl __scrt_initialize_onexit_tables(__scrt_module_type const module_type)
{
    if (module_type == __scrt_module_type::dll)
    {
        // If this module is a DLL, we initialize our module-local onexit table:
        if (_initialize_onexit_table(&module_local_atexit_table) != 0)
            return false;

        if (_initialize_onexit_table(&module_local_at_quick_exit_table) != 0)
            return false;

        return true;
    }
    else if (module_type == __scrt_module_type::exe)
    {
        // Initialize the onexit table pointers to indicate that this is an EXE,
        // not a DLL.  The EXE will register its onexit functions in the table
        // in the CRT instead of in its own table.  This ensures that they will
        // be called at the right time during the termination logic.
        _PVFV* const encoded_invalid = __crt_fast_encode_pointer(reinterpret_cast<_PVFV*>(-1));

        module_local_atexit_table        = { encoded_invalid, encoded_invalid, encoded_invalid };
        module_local_at_quick_exit_table = { encoded_invalid, encoded_invalid, encoded_invalid };

        return true;
    }
    else
    {
        __scrt_fastfail(FAST_FAIL_INVALID_ARG);
    }
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
    UNREFERENCED_PARAMETER(instance);
    UNREFERENCED_PARAMETER(reason);
    UNREFERENCED_PARAMETER(reserved);
    UNREFERENCED_PARAMETER(crt_dllmain);

    return _seh_filter_dll(exception_code_, exception_info_);
}

extern "C" bool __cdecl __scrt_dllmain_before_initialize_c()
{
    if (!__scrt_initialize_onexit_tables(__scrt_module_type::dll))
        return false;

    return true;
}

extern "C" bool __cdecl __scrt_dllmain_after_initialize_c()
{
    __isa_available_init();
    return true;
}

extern "C" void __cdecl __scrt_dllmain_uninitialize_c()
{
    _execute_onexit_table(&module_local_atexit_table);
}

extern "C" void __cdecl __scrt_dllmain_uninitialize_critical()
{
    // No action is required
}

extern "C" bool __cdecl __scrt_dllmain_crt_thread_attach()
{
    return true; // No action is required
}

extern "C" bool __cdecl __scrt_dllmain_crt_thread_detach()
{
    return true; // No action is required
}
