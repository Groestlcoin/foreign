//
// thread.cpp
//
//      Copyright (c) Microsoft Corporation. All rights reserved.
//
// Definitions of _beginthread() and _endthread(), which are used to begin and
// end execution of a thread.
//
#include <corecrt_internal.h>
#include <process.h>

// In some compilation models, the compiler is able to detect that the return
// statement at the end of thread_start is unreachable.  We cannot suppress the
// warning locally because it is a backend warning.
#pragma warning(disable: 4702) // unreachable code



static unsigned long WINAPI thread_start(void*) throw();



// Creates a new thread, with execution beginning by calling the given procedure
// with the given context pointer.  On success, returns a handle to the thread.
// On failure, returns INVALID_HANDLE_VALUE and sets errno and _doserrno.
extern "C" uintptr_t __cdecl _beginthread(
    __dcrt_thread_procedure const procedure,
    unsigned int            const stack_size,
    void*                   const context
    )
{
    _VALIDATE_RETURN(procedure != nullptr, EINVAL, static_cast<uintptr_t>(-1));

    // We need to initialize the AppCRT PTD in the new thread with the locale
    // state from this thread:
    __crt_unique_heap_ptr<__acrt_ptd, __crt_internal_acrt_freefls_policy> new_acrt_ptd(__acrt_copy_current_ptd_for_new_thread());
    if (!new_acrt_ptd)
        return reinterpret_cast<uintptr_t>(INVALID_HANDLE_VALUE);

    __crt_unique_heap_ptr<__dcrt_thread_parameter> parameter(_calloc_crt_t(__dcrt_thread_parameter, 1));
    if (!parameter)
        return reinterpret_cast<uintptr_t>(INVALID_HANDLE_VALUE);

    parameter.get()->_procedure = reinterpret_cast<void*>(procedure);
    parameter.get()->_context   = context;
    parameter.get()->_acrt_ptd  = new_acrt_ptd.get();

    // We create the new thread in a suspended state so that we can update
    // the parameter structure with the thread handle.  The newly created
    // thread is responsible for closing this handle
    DWORD thread_id;
    HANDLE const thread_handle = CreateThread(
        nullptr,
        stack_size,
        thread_start,
        parameter.get(),
        CREATE_SUSPENDED,
        &thread_id);

    if (thread_handle == nullptr)
    {
        __acrt_errno_map_os_error(GetLastError());
        return reinterpret_cast<uintptr_t>(INVALID_HANDLE_VALUE); 
    }

    parameter.get()->_thread_handle = thread_handle;

    // Now we can start the thread...
    if (ResumeThread(thread_handle) == static_cast<DWORD>(-1))
    {
        __acrt_errno_map_os_error(GetLastError());
        return reinterpret_cast<uintptr_t>(INVALID_HANDLE_VALUE); 
    }

    // If thread creation completed successfully, the other thread now owns
    // the PTD and parameter:
    parameter.detach();
    new_acrt_ptd.detach();

    return reinterpret_cast<uintptr_t>(thread_handle);
}



// The entry point for threads begun via _beginthread().
static unsigned long WINAPI thread_start(void* const untyped_parameter) throw()
{
    if (untyped_parameter == nullptr)
        ExitThread(GetLastError());

    __dcrt_thread_parameter* const parameter = 
        static_cast<__dcrt_thread_parameter*>(untyped_parameter);

    __acrt_ptd* const ptd = __acrt_setptd(parameter->_acrt_ptd);

    // If we got nullptr back, something has gone horribly wrong--we can't
    // create the PTD for this thread, and we can't do anything without a PTD.
    // So we simply exit the thread:
    if (ptd == nullptr)
    {
        __acrt_freefls(parameter->_acrt_ptd);
        _free_crt(parameter);
        ExitThread(GetLastError());
    }

    // If the call to set the PTD succeeded but we got a different PTD back,
    // the PTD has already been set.  This can happen e.g. when using the CRT
    // DLL, which will initialize the PTD during DLL_THREAD_ATTACH.  We just
    // need to clean up our PTD:
    if (ptd != parameter->_acrt_ptd)
    {
        __acrt_freefls(parameter->_acrt_ptd);
        parameter->_acrt_ptd = nullptr; // Just to be safe
    }

    // Otherwise, the PTD was initialized with our PTD.  Now we can create the
    // DesktopCRT PTD and initialize it:
    __dcrt_getptd()->_beginthread_context = parameter;

    // Guard the call to the user's thread entry point with a __try-__except to
    // implement runtime errors and signal support:
    __try
    {
        __dcrt_thread_procedure const typed_procedure =
            reinterpret_cast<__dcrt_thread_procedure>(parameter->_procedure);

        typed_procedure(parameter->_context);
        _endthread();
    }
    __except (_seh_filter_exe(GetExceptionCode(), GetExceptionInformation()))
    {
        // Execution should never reach here:
        _exit(GetExceptionCode());
    }

    // This return statement will never be reached.  All execution paths result
    // in the thread or process exiting.
    return 0;
}



// Terminates the calling thread after cleaning up per-thread CRT data
extern "C" void __cdecl _endthread()
{
    __dcrt_ptd* const ptd = __dcrt_getptd_noexit();
    if (ptd == nullptr)
        ExitThread(0);

    // Close the thread handle, if we own one:
    __dcrt_thread_parameter* const parameter = ptd->_beginthread_context;
    if (parameter != nullptr &&
        parameter->_thread_handle != INVALID_HANDLE_VALUE &&
        parameter->_thread_handle != nullptr)
    {
        CloseHandle(parameter->_thread_handle);
    }

    __dcrt_freeptd(ptd);
    _free_crt(parameter);

    ExitThread(0);
}
