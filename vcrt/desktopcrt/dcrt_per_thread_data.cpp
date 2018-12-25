//
// per_thread_data.cpp
//
//      Copyright (c) Microsoft Corporation. All rights reserved.
//
// Per-Thread Data (PTD) used by the DesktopCRT.
//
#include <corecrt_internal.h>
#include <stddef.h>



#ifdef _CRT_GLOBAL_STATE_ISOLATION
extern "C" DWORD __crt_global_state_mode_flsindex = FLS_OUT_OF_INDEXES;
#endif

static unsigned long __dcrt_flsindex = FLS_OUT_OF_INDEXES;



extern "C" bool __cdecl __dcrt_initialize_ptd()
{
    __dcrt_flsindex = __acrt_FlsAlloc(&__dcrt_freefls);
    if (__dcrt_flsindex == FLS_OUT_OF_INDEXES)
    {
        return false;
    }

    if (__dcrt_getptd_noexit() == nullptr)
    {
        __dcrt_uninitialize_ptd();
        return false;
    }

    return true;
}

extern "C" bool __cdecl __dcrt_uninitialize_ptd()
{
    if (__dcrt_flsindex != FLS_OUT_OF_INDEXES)
    {
        __acrt_FlsFree(__dcrt_flsindex);
        __dcrt_flsindex = FLS_OUT_OF_INDEXES;
    }

    return true;
}

static __forceinline __dcrt_ptd* get_ptd_from_ptd_head(__dcrt_ptd* const ptd_head)
{
    return ptd_head + __crt_state_management::get_current_state_index();
}

// This functionality has been split out of __dcrt_getptd_noexit so that we can
// force it tobe inlined into both __dcrt_getptd_noexit and __dcrt_getptd.  These
// functions are performance critical and this change has substantially improved
// __dcrt_getptd performance.
static __forceinline __dcrt_ptd* __cdecl internal_getptd_noexit() throw()
{
    // If we haven't allocated per-thread data for this module, return failure:
    if (__dcrt_flsindex == FLS_OUT_OF_INDEXES)
    {
        return nullptr; // Return nullptr to indicate failure
    }

    __crt_scoped_get_last_error_reset const last_error_reset;

    // First see if we've already created per-thread data for this thread:
    __dcrt_ptd* const existing_ptd = static_cast<__dcrt_ptd*>(__acrt_FlsGetValue(__dcrt_flsindex));
    if (existing_ptd)
    {
        return get_ptd_from_ptd_head(existing_ptd);
    }

    // No per-thread data for this thread yet.  Try to create one:
    __crt_unique_heap_ptr<__dcrt_ptd> ptd_head(_calloc_crt_t(__dcrt_ptd, __crt_state_management::state_index_count));
    if (!ptd_head)
    {
        return nullptr; // Return nullptr to indicate failure
    }

    if (!__acrt_FlsSetValue(__dcrt_flsindex, ptd_head.get()))
    {
        return nullptr; // Return nullptr to indicate failure
    }

    return get_ptd_from_ptd_head(ptd_head.detach());
}

extern "C" __dcrt_ptd* __cdecl __dcrt_getptd_noexit()
{
    return internal_getptd_noexit();
}

extern "C" __dcrt_ptd* __cdecl __dcrt_getptd()
{
    __dcrt_ptd* const ptd = internal_getptd_noexit();
    if (ptd == nullptr)
    {
        __acrt_report_runtime_error_and_exit(_RT_THREAD);
    }

    return ptd;
}

extern "C" __dcrt_ptd* __cdecl __dcrt_setptd(__dcrt_ptd* const _Ptd)
{
    if (__dcrt_flsindex == FLS_OUT_OF_INDEXES)
    {
        return nullptr; // Return nullptr to indicate complete and utter failure
    }

    __dcrt_ptd* const existing_ptd = static_cast<__dcrt_ptd*>(__acrt_FlsGetValue(__dcrt_flsindex));
    if (existing_ptd != nullptr)
    {
        return existing_ptd;
    }

    if (!__acrt_FlsSetValue(__dcrt_flsindex, _Ptd))
    {
        return nullptr;
    }

    return _Ptd;
}

extern "C" void __cdecl __dcrt_freeptd(__dcrt_ptd* const _Ptd)
{
    // Do nothing unless per-thread data has been allocated for this module:
    if (__dcrt_flsindex == FLS_OUT_OF_INDEXES)
        return;

    // If the argument is null, get the pointer for this thread.  Note that we
    // must not call __dcrt_getptd, because it will allocate a new per-thread
    // data if one does not already exist.
    __dcrt_ptd* const block_to_free = _Ptd == nullptr
        ? static_cast<__dcrt_ptd*>(__acrt_FlsGetValue(__dcrt_flsindex))
        : _Ptd;

    // If we never allocated PTD for this thread, we have no cleanup to do:
    if (block_to_free == nullptr)
        return;

    __acrt_FlsSetValue(__dcrt_flsindex, nullptr);
    __dcrt_freefls(block_to_free);
}

// This function is called by the operating system when a thread is being
// destroyed, to allow us the opportunity to clean up.
extern "C" void WINAPI __dcrt_freefls(void* const _Pfd)
{
    // No cleanup is currently required; freeing is sufficient.
    _free_crt(_Pfd);
}
