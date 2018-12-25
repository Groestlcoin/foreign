//
// threadex.cpp
//
//      Copyright (c) Microsoft Corporation. All rights reserved.
//
// Definitions of _beginthreadex() and _endthreadex(), which are used to begin
// and end execution of a thread.  These functions are more like the Windows API
// functions CreateThread() and ExitThread() than the original CRT functions
// _beginthread() and _endthread().
//
#include <corecrt_internal.h>
#include <process.h>

// In some compilation models, the compiler is able to detect that the return
// statement at the end of thread_start is unreachable.  We cannot suppress the
// warning locally because it is a backend warning.
#pragma warning(disable: 4702) // unreachable code



static unsigned long WINAPI thread_start(void*) throw();



// Flags to be passed to Windows::Runtime::Initialize
typedef enum RO_INIT_TYPE
{
    RO_INIT_SINGLETHREADED = 0,
    RO_INIT_MULTITHREADED  = 1,
} RO_INIT_TYPE;



// CRT_REFACTOR TODO These don't look right... let's just leak HMODULEs?  Also,
// volatile != atomic.
static int initialize_mta_on_current_thread() throw()
{
    static void * volatile s_roinit;
    static volatile int s_initialized;

    if (!s_initialized)
    {
        void *pfn = GetProcAddress(LoadLibraryExW(L"combase.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32), "RoInitialize");
        if (pfn == nullptr)
            return 0;

        s_roinit = EncodePointer(pfn);
        s_initialized = 1;
    }
    return ((HRESULT (WINAPI *) (RO_INIT_TYPE)) DecodePointer(s_roinit)) (RO_INIT_MULTITHREADED) == S_OK;
}

static void uninitialize_mta_on_current_thread() throw()
{
    static void * volatile s_rouninit;
    static volatile int s_initialized;

    if (!s_initialized)
    {
        void *pfn = GetProcAddress(LoadLibraryExW(L"combase.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32), "RoUninitialize");
        if (pfn == nullptr)
            return;
        s_rouninit = EncodePointer(pfn);
        s_initialized = 1;
    }
    ((void (WINAPI *)()) DecodePointer(s_rouninit))();
}



// Creates a new thread, with execution beginning by calling the given procedure
// with the given context pointer.  On success, returns a handle to the thread
// and sets the thread_id_result if it is non-null.  On failure, returns 0 and
// sets errno.
//
// This routine is more similar to the Windows API CreateThread() than the older
// _beginthread() is.  There are several differences between _beginthread() and
// _beginthreadex():
//
//  * _beginthreadex() takes three additional parameters, which are passed on to
//    CreateThread():  a security descriptor for the new thread, the initial
//    thread state (running or asleep), and an optional out parameer to which
//    the thread id of the newly created thread will be stored.
//
//  * The procedure passed to _beginthread() must be __cdecl and has no return
//    code.  The routine passed to _beginthreadex() must be __stdcall and must
//    return a return code, which will be used as the thread exit code.
//    Likewise, _endthread() takes no parameter and always returns a thread exit
//    code of 0 if the thread exits without error, whereas _endthreadex() takes
//    an exit code.
//
//  * _endthread() calls CloseHandle() on the handle returned from CReateThread().
//    Note that this means that a caller should not use this handle, since it is
//    possible that the thread will have terminated and the handle will have been
//    closed by the time that _beginthread() returns.
//
//    _endthreadex() does not call CloseHandle() to close the handle:  the caller
//    of _beginthreadex() is required to close the handle.
//
//  * _beginthread() returns -1 on failure.  _beginthreadex() returns zero on
//    failure (just as CreateThread() does).
//
//  * When called in a packaged app, _beginthreadex() initializes the WinRT
//    apartment for the thread (and _endthreadex() uninitializes it).
extern "C" uintptr_t __cdecl _beginthreadex(
    void*                      const security_descriptor,
    unsigned int               const stack_size,
    __dcrt_thread_procedure_ex const procedure,
    void*                      const context,
    unsigned                   const creation_flags,
    unsigned int*              const thread_id_result
    )
{
    _VALIDATE_RETURN(procedure != nullptr, EINVAL, 0);
    // We need to initialize the AppCRT PTD in the new thread with the locale
    // state from this thread:
    __crt_unique_heap_ptr<__acrt_ptd, __crt_internal_acrt_freefls_policy> new_acrt_ptd(__acrt_copy_current_ptd_for_new_thread());
    if (!new_acrt_ptd)
        return 0;

    __crt_unique_heap_ptr<__dcrt_thread_parameter> parameter(_calloc_crt_t(__dcrt_thread_parameter, 1));
    if (!parameter)
        return 0;

    parameter.get()->_procedure = reinterpret_cast<void*>(procedure);
    parameter.get()->_context   = context;
    parameter.get()->_acrt_ptd  = new_acrt_ptd.get();

    DWORD thread_id;
    HANDLE const thread_handle = CreateThread(
        reinterpret_cast<LPSECURITY_ATTRIBUTES>(security_descriptor),
        stack_size,
        thread_start,
        parameter.get(),
        creation_flags,
        &thread_id);

    if (thread_handle == nullptr)
    {
        __acrt_errno_map_os_error(GetLastError());
        return 0;
    }

    if (thread_id_result != nullptr)
        *thread_id_result = thread_id;

    // If everything completed successfully, then the other thread now has
    // ownership of the PTD and parameter:
    parameter.detach();
    new_acrt_ptd.detach();

    return reinterpret_cast<uintptr_t>(thread_handle);
}



// The entry point for threads begun via _beginthreadex();
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

    // If we're loaded in a packaged app, we need to initialize the apartment:
    parameter->_initialized_apartment = __acrt_IsPackagedApp();
    if (parameter->_initialized_apartment)
        parameter->_initialized_apartment = initialize_mta_on_current_thread();

    __try
    {
        __dcrt_thread_procedure_ex const typed_procedure =
            reinterpret_cast<__dcrt_thread_procedure_ex>(parameter->_procedure);

        _endthreadex(typed_procedure(parameter->_context));
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
extern "C" void __cdecl _endthreadex(unsigned const return_code)
{
    __dcrt_ptd* const ptd = __dcrt_getptd_noexit();
    if (ptd == nullptr)
        ExitThread(return_code);

    __dcrt_thread_parameter* const parameter = ptd->_beginthread_context;
    if (parameter == nullptr)
        ExitThread(return_code);

    if (parameter->_initialized_apartment)
        uninitialize_mta_on_current_thread();

    __dcrt_freeptd(ptd);
    _free_crt(parameter);

    ExitThread(return_code);
}
