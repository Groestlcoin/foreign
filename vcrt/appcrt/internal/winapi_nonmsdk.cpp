//
// winapi_nonmsdk.cpp
//
//      Copyright (c) Microsoft Corporation. All rights reserved.
//
// AppCRT Non-MSDK Windows API Dependencies.
//
#include <corecrt_internal.h>

namespace {

// This enumeration declares enumerator identifiers for all of the modules from
// which we dynamically import functions.
enum module_id : unsigned
{
    #define _APPLY(_M, _N) _CRT_CONCATENATE(module_id_, _M),
    _ACRT_APPLY_TO_NONMSDK_MODULES(_APPLY)
    #undef _APPLY

    // The last enumerator is the number of modules:
    module_id_count
};

// In the AppCRT, create an alias for the debug DesktopCRT module id so that we
// can refer to it using 'module_id_desktopcrt' elsewhere in this file.
#ifdef _DEBUG
    module_id const module_id_desktopcrt140 = module_id_desktopcrt140d;
#endif

static wchar_t const* const module_names[module_id_count] =
{
    #define _APPLY(_M, _N) _CRT_WIDE(_N),
    _ACRT_APPLY_TO_NONMSDK_MODULES(_APPLY)
    #undef _APPLY
};

// Generate typedefs for each function 'func', named 'func_pft'
#define _APPLY(_M, _Z, _R, _C, _N, _PL, _AL) \
    typedef _R (_C * _N ## _pft) _PL;
_ACRT_APPLY_TO_NONMSDK_FUNCTIONS(_APPLY)
#undef _APPLY

// This structure has as data members pointers to each function we import:
struct function_table
{
    // Generate function pointer members, named func_pf
    #define _APPLY(_M, _Z, _R, _C, _N, _PL, _AL) \
        volatile _N ## _pft _N ## _pf;
    _ACRT_APPLY_TO_NONMSDK_FUNCTIONS(_APPLY);
    #undef _APPLY
};

} // unnamed namespace

extern "C" {

// This table stores the module handles that we have LoadLibrary'ed:
static volatile HMODULE __acrt_module_handles[module_id_count];

// This is the global table of function pointers:
static volatile function_table __acrt_winapi_functions;



// CRT_REFACTOR TODO We need to add the ability to detect the difference between
// debug CRT initialization and desktop CRT initialization.
bool __cdecl __acrt_has_desktop_support()
{
    auto const cache_storage = __acrt_module_handles + _LIBRARYLOADER_MODULE_ID;

    return __crt_fast_decode_pointer(__crt_interlocked_read_pointer(cache_storage)) != nullptr;
}

bool __cdecl __acrt_has_debug_support()
{
    #ifdef _DEBUG
    return true;
    #else
    return false;
    #endif
}

bool __cdecl __acrt_has_desktop_or_debug_support()
{
    return __acrt_has_desktop_support() || __acrt_has_debug_support();
}



// Gets a module handle from the cache.  If the handle for that module has not
// yet been cached, this function loads the module and caches the handle, then
// returns the newly cached handle.
static HMODULE __cdecl get_or_load_module_handle(module_id const id)
{
    auto const cache_storage = __acrt_module_handles + id;

    // First see if we've already cached the module handle; if we have, return it:
    if (HMODULE const handle = __crt_fast_decode_pointer(__crt_interlocked_read_pointer(cache_storage)))
        return handle;

    // Okay, we haven't cached it.  Try to load the module:
    auto const load_library = __crt_fast_decode_pointer(__acrt_winapi_functions.LoadLibraryExW_pf);
    _ASSERT_AND_INVOKE_WATSON(load_library != nullptr);

    auto const our_handle = load_library(module_names[id], nullptr, 0);
    _ASSERT_AND_INVOKE_WATSON(our_handle != nullptr);

    // We've successfully loaded the module, so we can cache the handle.  If
    // the cached handle is now non-null, some other thread loaded the module
    // already and cached the handle.  In this case, we need to call FreeLibrary
    // to release our reference to the module:
    if (HMODULE const cached_handle = __crt_fast_decode_pointer(
            __crt_interlocked_compare_exchange_pointer(
                cache_storage,
                __crt_fast_encode_pointer(our_handle),
                nullptr)))
    {
        _ASSERT_AND_INVOKE_WATSON(cached_handle == our_handle);

        auto const free_library = __crt_fast_decode_pointer(__acrt_winapi_functions.FreeLibrary_pf);
        _ASSERT_AND_INVOKE_WATSON(free_library != nullptr);

        free_library(cached_handle);
    }

    return our_handle;
}

#define GET_PROC_ADDRESS(_ModuleHandle, _ProcName) \
    (reinterpret_cast<_ProcName ## _pft>(GetProcAddress(_ModuleHandle, # _ProcName)))

// This generates functions that get function pointers from the cache.  If the
// function pointer has not yet been cached, this function calls GetProcAddress
// to get the function pointer then caches and returns it.
//
// For lazily-initialized function pointers, we need to synchronize access to
// the cache.  For eagerly-initialized function pointers, no synchronization is
// required.
#define _APPLY_LAZY(_M, _Z, _R, _C, _N, _PL, _AL)                                                          \
    static _N ## _pft __cdecl get_cached_ ## _N ## _pf()                                                   \
    {                                                                                                      \
        auto cache_storage = &__acrt_winapi_functions._N ## _pf;                                           \
                                                                                                           \
        if (auto pf = __crt_fast_decode_pointer(__crt_interlocked_read_pointer(cache_storage)))            \
            return pf;                                                                                     \
                                                                                                           \
        HMODULE const handle = get_or_load_module_handle(module_id_ ## _M);                                \
        auto const pf = GET_PROC_ADDRESS(handle, _N);                                                      \
        _ASSERT_AND_INVOKE_WATSON(pf != nullptr);                                                          \
                                                                                                           \
        __crt_interlocked_compare_exchange_pointer(cache_storage, __crt_fast_encode_pointer(pf), nullptr); \
        return pf;                                                                                         \
    }

#define _APPLY_EAGER(_M, _Z, _R, _C, _N, _PL, _AL)                                     \
    static _N ## _pft __cdecl get_cached_ ## _N ## _pf()                               \
    {                                                                                  \
        auto const pf = __crt_fast_decode_pointer(__acrt_winapi_functions._N ## _pf);  \
        _ASSERT_AND_INVOKE_WATSON(pf != nullptr);                                      \
        return pf;                                                                     \
    }

#define _APPLY(_M, _Z, _R, _C, _N, _PL, _AL) \
    _APPLY_ ## _Z(_M, _Z, _R, _C, _N, _PL, _AL)
_ACRT_APPLY_TO_NONMSDK_FUNCTIONS(_APPLY)
#undef _APPLY
#undef _APPLY_EAGER
#undef _APPLY_LAZY



bool __cdecl __acrt_pre_initialize_nonmsdk_dependencies()
{
    auto const encoded_nullptr = __crt_fast_encode_pointer(nullptr);

    #define _APPLY(_M, _Z, _R, _C, _N, _PL, _AL) \
        __acrt_winapi_functions._N ## _pf = encoded_nullptr;
    _ACRT_APPLY_TO_NONMSDK_FUNCTIONS(_APPLY);
    #undef _APPLY

    for (size_t i = 0; i != module_id_count; ++i)
    {
        __acrt_module_handles[i] = encoded_nullptr;
    }

    return true;
}



bool __cdecl __acrt_post_initialize_nonmsdk_dependencies(HMODULE const _LibraryLoaderHandle)
{
    _ASSERT_AND_INVOKE_WATSON(_LibraryLoaderHandle != nullptr);

    // First, let's get the address of LoadLibrary and LoadLibrary the library loader DLL:
    auto const load_library = GET_PROC_ADDRESS(_LibraryLoaderHandle, LoadLibraryExW);
    _ASSERT_AND_INVOKE_WATSON(load_library != nullptr);

    auto const library_loader_handle = load_library(_CRT_WIDE(_LIBRARYLOADER_MODULE_NAME), nullptr, 0);
    _ASSERT_AND_INVOKE_WATSON(library_loader_handle == _LibraryLoaderHandle);

    __acrt_module_handles[_LIBRARYLOADER_MODULE_ID] = __crt_fast_encode_pointer(library_loader_handle);
    __acrt_winapi_functions.LoadLibraryExW_pf = __crt_fast_encode_pointer(load_library);

    // Next, let's get the HMODULE for the DesktopCRT.  We handle the DesktopCRT
    // specially because we do not want to increment its reference count.  If we
    // held a strong reference to DesktopCRT, we would cause a reference cycle
    // between the two modules--it has a load-time dependency on us (the AppCRT)
    // and we would have a strong reference back to it.
    //
    // We synchronize both startup and shutdown of the various CRT modules, so
    // the DesktopCRT will notify us before it is shut down and unloaded; thus,
    // we don't need to hold a strong reference to it; we'll be able to clean up
    // when it explicitly notifies us that we need to do so.
    //
    // (Note that there is no opportunity for race here, because this function
    // is called under the loader lock.
    auto const get_module_handle = GET_PROC_ADDRESS(_LibraryLoaderHandle, GetModuleHandleExW);
    _ASSERT_AND_INVOKE_WATSON(get_module_handle != nullptr);

    #ifdef _CRTDLL
    HMODULE desktopcrt_handle = nullptr;
    get_module_handle(
        GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
        _CRT_WIDE(_CRT_STRINGIZE(_DESKTOPCRT_MODULE)) L".dll",
        &desktopcrt_handle);

    _ASSERT_AND_INVOKE_WATSON(
        desktopcrt_handle != INVALID_HANDLE_VALUE &&
        desktopcrt_handle != nullptr);

    __acrt_module_handles[module_id_desktopcrt] = __crt_fast_encode_pointer(desktopcrt_handle);
    #endif

    // Now we can eagerly initialize all of the function pointers that were
    // marked for preinitialization when they were declared in the list.
    #define _APPLY_LAZY(_M, _Z, _R, _C, _N, _PL, _AL)
    #define _APPLY_EAGER(_M, _Z, _R, _C, _N, _PL, _AL)                            \
    {                                                                             \
        HMODULE const handle = get_or_load_module_handle(module_id_ ## _M);       \
        auto const pf = GET_PROC_ADDRESS(handle, _N);                             \
        _ASSERT_AND_INVOKE_WATSON(pf != nullptr);                                 \
        __acrt_winapi_functions._N ## _pf = __crt_fast_encode_pointer(pf);        \
    }

    #define _APPLY(_M, _Z, _R, _C, _N, _PL, _AL) \
        _APPLY_ ## _Z(_M, _Z, _R, _C, _N, _PL, _AL)

    _ACRT_APPLY_TO_NONMSDK_FUNCTIONS(_APPLY);

    #undef _APPLY
    #undef _APPLY_EAGER
    #undef _APPLY_LAZY

    return true;
}

// CRT_REFACTOR TODO The static debug CRT may call non-MSDK APIs during process
// exit after it calls __acrt_uninitialize_nonmsdk_dependencies().  We should
// probably either [1] disable the non-MSDK thunks for the static CRT since it
// cannot be used in a Windows Store app, or [2] remove all of the thunks once
// we get permission to use non-MSDK APIs again.  In the meantime, we simply
// disable __acrt_uninitialize_nonmsdk_dependencies()'s behavior in the static
// debug CRT.
#if defined CRTDLL || !defined _DEBUG

    bool __cdecl __acrt_uninitialize_nonmsdk_dependencies(bool const /* terminating */)
    {
        // We'll need the pointer to FreeLibrary in order to free the module handles
        // after we set all of the function pointers to null.
        auto const free_library = __crt_fast_decode_pointer(__acrt_winapi_functions.FreeLibrary_pf);

        // We always initialize FreeLibrary during initialization, so if it is null
        // either we never initialized the table or we have already uninitialized the
        // table.  That's okay; we can just return:
        if (free_library == nullptr)
            return true;

        auto const encoded_nullptr = __crt_fast_encode_pointer(nullptr);

        // Set all of the global function pointers to nullptr:
        #define _APPLY(_M, _Z, _R, _C, _N, _PL, _AL) \
            __acrt_winapi_functions._N ## _pf = encoded_nullptr;
        _ACRT_APPLY_TO_NONMSDK_FUNCTIONS(_APPLY);
        #undef _APPLY

        // Free all of the stored module handles except the library loader (which we need
        // because it exports FreeLibrary, which we are using).
        for (size_t i = 0; i != module_id_count; ++i)
        {
            if (i == _LIBRARYLOADER_MODULE_ID)
                continue;

            if (__acrt_module_handles[i] == encoded_nullptr)
                continue;

            // Free the module handle unless it is the DesktopCRT module handle. See
            // the logic in __acrt_post_initialize_nonmsdk_dependencies that handles
            // the DesktopCRT specially for an explanation of why.
            if (i != module_id_desktopcrt140)
                free_library(__crt_fast_decode_pointer(__acrt_module_handles[i]));

            __acrt_module_handles[i] = encoded_nullptr;
        }

        // Finally, free the library loader handle:
        free_library(__crt_fast_decode_pointer(__acrt_module_handles[_LIBRARYLOADER_MODULE_ID]));
        __acrt_module_handles[_LIBRARYLOADER_MODULE_ID] = encoded_nullptr;

        return true;
    }

#else // ^^^ defined CRTDLL || !defined _DEBUG ^^ // vvv !defined CRTDLL && defined _DEBUG vvv //

    bool __cdecl __acrt_uninitialize_nonmsdk_dependencies(bool const /* terminating */)
    {
        return true;
    }

#endif

// This generates the wrapper function definitions.  Each wrapper function calls
// get_cached_func_pf() to get the function pointer, then calls that function
// with the provided arguments and returns the result.
#define _APPLY(_M, _Z, _R, _C, _N, _PL, _AL)        \
    _R __acrt_ ## _N _PL                            \
    {                                               \
        auto const pf = get_cached_ ## _N ## _pf(); \
        return pf _AL;                              \
    }
_ACRT_APPLY_TO_NONMSDK_FUNCTIONS(_APPLY)
#undef _APPLY

} // extern "C"
