//
// per_thread_data.cpp
//
//      Copyright (c) Microsoft Corporation. All rights reserved.
//
// Per-Thread Data (PTD) used by the VCRuntime.
//
#include <vcruntime_internal.h>



static unsigned long __vcrt_flsindex = FLS_OUT_OF_INDEXES;

// In order to avoid dynamic allocation during CRT startup, the PTD for the
// startup thread is statically allocated:
static __vcrt_ptd __vcrt_startup_thread_ptd;



static bool __cdecl store_and_initialize_ptd(__vcrt_ptd* const ptd)
{
    if (!__vcrt_FlsSetValue(__vcrt_flsindex, ptd))
        return false;

    // Zero-initialization of the PTD is sufficient.
    return true;
}



extern "C" bool __cdecl __vcrt_initialize_ptd()
{
    __vcrt_flsindex = __vcrt_FlsAlloc(&__vcrt_freefls);
    if (__vcrt_flsindex == FLS_OUT_OF_INDEXES)
    {
        return false;
    }

    if (!store_and_initialize_ptd(&__vcrt_startup_thread_ptd))
    {
        __vcrt_uninitialize_ptd();
        return false;
    }

    return true;
}

extern "C" bool __cdecl __vcrt_uninitialize_ptd()
{
    if (__vcrt_flsindex != FLS_OUT_OF_INDEXES)
    {
        __vcrt_FlsFree(__vcrt_flsindex);
        __vcrt_flsindex = FLS_OUT_OF_INDEXES;
    }

    return true;
}

extern "C" __vcrt_ptd* __cdecl __vcrt_getptd_noexit()
{
    // If we haven't allocated per-thread data for this module, return failure:
    if (__vcrt_flsindex == FLS_OUT_OF_INDEXES)
    {
        return nullptr; // Return nullptr to indicate failure
    }

    DWORD const old_last_error = GetLastError();

    // First see if we've already created per-thread data for this thread:
    __vcrt_ptd* const existing_ptd = static_cast<__vcrt_ptd*>(__vcrt_FlsGetValue(__vcrt_flsindex));
    if (existing_ptd != nullptr)
    {
        SetLastError(old_last_error);
        return existing_ptd;
    }

    // No per-thread data for this thread yet.  Try to create one:
    __crt_unique_heap_ptr<__vcrt_ptd> new_ptd(_calloc_crt_t(__vcrt_ptd, 1));
    if (!new_ptd)
    {
        SetLastError(old_last_error);
        return nullptr; // Return nullptr to indicate failure
    }

    if (!store_and_initialize_ptd(new_ptd.get()))
    {
        SetLastError(old_last_error);
        return nullptr; // Return nullptr to indicate failure
    }

    SetLastError(old_last_error);
    return new_ptd.detach();
}

extern "C" __vcrt_ptd* __cdecl __vcrt_getptd()
{
    __vcrt_ptd* const ptd = __vcrt_getptd_noexit();
    if (ptd == nullptr)
    {
        abort();
    }

    return ptd;
}

extern "C" void __cdecl __vcrt_freeptd(_Inout_opt_ __vcrt_ptd* const ptd)
{
    // Do nothing unless per-thread data has been allocated for this module:
    if (__vcrt_flsindex == FLS_OUT_OF_INDEXES)
        return;

    // If the argument is null, get the pointer for this thread.  Note that we
    // must not call __vcrt_getptd, because it will allocate a new per-thread
    // data if one does not already exist.
    __vcrt_ptd* const block_to_free = ptd == nullptr
        ? static_cast<__vcrt_ptd*>(__vcrt_FlsGetValue(__vcrt_flsindex))
        : ptd;

    __vcrt_FlsSetValue(__vcrt_flsindex, nullptr);
    __vcrt_freefls(block_to_free);
}

// This function is called by the operating system when a thread is being
// destroyed, to allow us the opportunity to clean up.
extern "C" void WINAPI __vcrt_freefls(_Inout_opt_ void* const pfd)
{
    if (pfd == nullptr || pfd == &__vcrt_startup_thread_ptd)
        return;
    
    _free_crt(pfd);
}
