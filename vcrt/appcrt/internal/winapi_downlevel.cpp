//
// winapi_downlevel.cpp
//
//      Copyright (c) Microsoft Corporation. All rights reserved.
//
// Definitions of wrappers for Windows API functions that are not available on
// all supported operating system versions.
//

#include <corecrt_internal.h>


#if _CORECRT_WIN32_WINNT < _WIN32_WINNT_WIN8

// This is the list of functions for which we cache function pointers to.
// Each element is a 2-tuple with the the name of the module from which the
// function is to be imported from, followed by the name of the function.
//
// For example:
//        _M        _N      
// _APPLY(KERNEL32, FlsFree)
#define _ACRT_APPLY_TO_DOWNLEVEL_CACHED_KERNEL32_FUNCTIONS(_APPLY)                 \
    _APPLY(kernel32, CompareStringEx)                                              \
    _APPLY(kernel32, EnumSystemLocalesEx)                                          \
    _APPLY(kernel32, FlsAlloc)                                                     \
    _APPLY(kernel32, FlsFree)                                                      \
    _APPLY(kernel32, FlsGetValue)                                                  \
    _APPLY(kernel32, FlsSetValue)                                                  \
    _APPLY(kernel32, GetFileInformationByHandleEx)                                 \
    _APPLY(kernel32, GetLocaleInfoEx)                                              \
    _APPLY(kernel32, GetSystemTimePreciseAsFileTime)                               \
    _APPLY(kernel32, InitializeCriticalSectionEx)                                  \
    _APPLY(kernel32, IsValidLocaleName)                                            \
    _APPLY(kernel32, GetDateFormatEx)                                              \
    _APPLY(kernel32, GetTimeFormatEx)                                              \
    _APPLY(kernel32, GetUserDefaultLocaleName)                                     \
    _APPLY(kernel32, LCIDToLocaleName)                                             \
    _APPLY(kernel32, LCMapStringEx)                                                \
    _APPLY(kernel32, LocaleNameToLCID)                                             \
    _APPLY(kernel32, SetThreadStackGuarantee)


namespace {

// Generate typedefs for each function 'func', named 'func_pft'
#define _APPLY(_M, _N)                                                             \
    typedef decltype(&_N) _N ## _pft;
_ACRT_APPLY_TO_DOWNLEVEL_CACHED_KERNEL32_FUNCTIONS(_APPLY)
#undef _APPLY

// This structure has as data members pointers to each function we import:
struct function_table
{
    // Generate function pointer members, named func_pf
    #define _APPLY(_M, _N)                                                         \
        volatile _N ## _pft _N ## _pf;
    _ACRT_APPLY_TO_DOWNLEVEL_CACHED_KERNEL32_FUNCTIONS(_APPLY);
    #undef _APPLY
};

} // unnamed namespace


// This stores the handle to kernel32:
extern "C" static volatile HMODULE __acrt_kernel32_module_handle;

// This is the global table of function pointers:
extern "C" static volatile function_table __acrt_downlevel_winapi_functions;

// Gets a module handle to kernel32 from the cache. If the handle for the module has not
// yet been cached, this function loads the module and caches the handle, then
// returns the newly cached handle.
static HMODULE __cdecl get_kernel32_module_handle() throw()
{
    auto const cache_storage = &__acrt_kernel32_module_handle;

    // First see if we've already cached the module handle; if we have, return it:
    if (HMODULE const handle = __crt_interlocked_exchange_pointer(cache_storage, nullptr))
    {
        return handle;
    }

    // We haven't cached the kernel32 handle, get the module handle:
    auto const our_handle = GetModuleHandleW(L"kernel32");
    _ASSERT_AND_INVOKE_WATSON(our_handle != nullptr);

    // We've successfully loaded the module, so we can cache the handle. If
    // the cached handle is now non-null, some other thread loaded the module
    // already and cached the handle:
    if (HMODULE const cached_handle = __crt_interlocked_compare_exchange_pointer(cache_storage, our_handle, nullptr))
    {
        _ASSERT_AND_INVOKE_WATSON(cached_handle == our_handle);
    }

    return our_handle;
}

#define GET_PROC_ADDRESS(_ModuleHandle, _ProcName)                                 \
    (reinterpret_cast<_ProcName ## _pft>(GetProcAddress(_ModuleHandle, # _ProcName)))

// This generates functions that get function pointers from the cache.
#define _APPLY(_M, _N)                                                                          \
    extern "C" _N ## _pft __cdecl __acrt_get_cached_ ## _N ## _pf()                             \
    {                                                                                           \
        auto const pf = __crt_fast_decode_pointer(__acrt_downlevel_winapi_functions._N ## _pf); \
        return pf;                                                                              \
    }
_ACRT_APPLY_TO_DOWNLEVEL_CACHED_KERNEL32_FUNCTIONS(_APPLY)
#undef _APPLY


#define _ACRT_CALL_FUNCTION_AND_RETURN_IF_AVAILABLE(functionname, ...)             \
    auto const pf = __acrt_get_cached_ ## functionname ## _pf();                   \
    if (pf != nullptr)                                                             \
    {                                                                              \
        return pf(__VA_ARGS__);                                                    \
    }

#endif // _CORECRT_WIN32_WINNT < _WIN32_WINNT_WIN8


extern "C" void __cdecl __acrt_initialize_downlevel_dependencies()
{
#if _CORECRT_WIN32_WINNT < _WIN32_WINNT_WIN8
    // Eagerly initialize all of the function pointers that were
    // marked for preinitialization when they were declared in the list.
    #define _APPLY(_M, _N)                                                           \
    {                                                                                \
        HMODULE const handle = get_kernel32_module_handle();                         \
        auto const pf = GET_PROC_ADDRESS(handle, _N);                                \
        __acrt_downlevel_winapi_functions._N ## _pf = __crt_fast_encode_pointer(pf); \
    }

    _ACRT_APPLY_TO_DOWNLEVEL_CACHED_KERNEL32_FUNCTIONS(_APPLY);

    #undef _APPLY
#endif
}

// Tests whether the pre-Vista APIs are used in the AppCRT.
extern "C" bool __cdecl __acrt_is_using_pre_vista_apis()
{
#if _CORECRT_WIN32_WINNT >= _WIN32_WINNT_VISTA
    return false;
#else
    auto const fp = __acrt_get_cached_InitializeCriticalSectionEx_pf();
    return fp == nullptr;
#endif
}


extern "C" BOOL __cdecl __acrt_SetThreadStackGuarantee(PULONG const StackSizeInBytes)
{
#if _CORECRT_WIN32_WINNT >= _WIN32_WINNT_VISTA
    return SetThreadStackGuarantee(StackSizeInBytes);
#else // _CORECRT_WIN32_WINNT >= _WIN32_WINNT_VISTA
    _ACRT_CALL_FUNCTION_AND_RETURN_IF_AVAILABLE(SetThreadStackGuarantee, StackSizeInBytes)    
    return FALSE;
#endif // _CORECRT_WIN32_WINNT >= _WIN32_WINNT_VISTA
}

extern "C" BOOL __cdecl __acrt_InitializeCriticalSectionEx(
    LPCRITICAL_SECTION const lpCriticalSection,
    DWORD              const dwSpinCount,
    DWORD              const Flags
    )
{
#if _CORECRT_WIN32_WINNT >= _WIN32_WINNT_VISTA
    return InitializeCriticalSectionEx(lpCriticalSection, dwSpinCount, Flags);
#else // _CORECRT_WIN32_WINNT >= _WIN32_WINNT_VISTA
    _ACRT_CALL_FUNCTION_AND_RETURN_IF_AVAILABLE(InitializeCriticalSectionEx, lpCriticalSection, dwSpinCount, Flags)
    return InitializeCriticalSectionAndSpinCount(lpCriticalSection, dwSpinCount);
#endif // _CORECRT_WIN32_WINNT >= _WIN32_WINNT_VISTA
}

extern "C" int __cdecl __acrt_GetLocaleInfoEx(
    LPCWSTR const lpLocaleName,
    LCTYPE  const LCType,
    LPWSTR  const lpLCData,
    int     const cchData
    )
{
#if _CORECRT_WIN32_WINNT >= _WIN32_WINNT_VISTA
    return GetLocaleInfoEx(lpLocaleName, LCType, lpLCData, cchData);
#else // _CORECRT_WIN32_WINNT >= _WIN32_WINNT_VISTA
    _ACRT_CALL_FUNCTION_AND_RETURN_IF_AVAILABLE(GetLocaleInfoEx, lpLocaleName, LCType, lpLCData, cchData)
    return GetLocaleInfoW(__acrt_DownlevelLocaleNameToLCID(lpLocaleName), LCType, lpLCData, cchData);
#endif // _CORECRT_WIN32_WINNT >= _WIN32_WINNT_VISTA
}

extern "C" DWORD __cdecl __acrt_FlsAlloc(PFLS_CALLBACK_FUNCTION const lpCallback)
{
#if _CORECRT_WIN32_WINNT >= _WIN32_WINNT_VISTA
    return FlsAlloc(lpCallback);
#else // _CORECRT_WIN32_WINNT >= _WIN32_WINNT_VISTA
    _ACRT_CALL_FUNCTION_AND_RETURN_IF_AVAILABLE(FlsAlloc, lpCallback)
    return TlsAlloc();
#endif // _CORECRT_WIN32_WINNT >= _WIN32_WINNT_VISTA
}

extern "C" BOOL __cdecl __acrt_FlsFree(DWORD const dwFlsIndex)
{
#if _CORECRT_WIN32_WINNT >= _WIN32_WINNT_VISTA
    return FlsFree(dwFlsIndex);
#else // _CORECRT_WIN32_WINNT >= _WIN32_WINNT_VISTA
    _ACRT_CALL_FUNCTION_AND_RETURN_IF_AVAILABLE(FlsFree, dwFlsIndex)
    return TlsFree(dwFlsIndex);
#endif // _CORECRT_WIN32_WINNT >= _WIN32_WINNT_VISTA
}

extern "C" PVOID __cdecl __acrt_FlsGetValue(DWORD const dwFlsIndex)
{
#if _CORECRT_WIN32_WINNT >= _WIN32_WINNT_VISTA
    return FlsGetValue(dwFlsIndex);
#else // _CORECRT_WIN32_WINNT >= _WIN32_WINNT_VISTA
    _ACRT_CALL_FUNCTION_AND_RETURN_IF_AVAILABLE(FlsGetValue, dwFlsIndex)
    return TlsGetValue(dwFlsIndex);
#endif // _CORECRT_WIN32_WINNT >= _WIN32_WINNT_VISTA
}

extern "C" BOOL __cdecl __acrt_FlsSetValue(DWORD const dwFlsIndex, PVOID const lpFlsData)
{
#if _CORECRT_WIN32_WINNT >= _WIN32_WINNT_VISTA
    return FlsSetValue(dwFlsIndex, lpFlsData);
#else // _CORECRT_WIN32_WINNT >= _WIN32_WINNT_VISTA
    _ACRT_CALL_FUNCTION_AND_RETURN_IF_AVAILABLE(FlsSetValue, dwFlsIndex, lpFlsData)
    return TlsSetValue(dwFlsIndex, lpFlsData);
#endif // _CORECRT_WIN32_WINNT >= _WIN32_WINNT_VISTA
}

extern "C" BOOL __cdecl __acrt_IsValidLocaleName(LPCWSTR const lpLocaleName)
{
 #if _CORECRT_WIN32_WINNT >= _WIN32_WINNT_VISTA
    return IsValidLocaleName(lpLocaleName);
#else // _CORECRT_WIN32_WINNT >= _WIN32_WINNT_VISTA
    _ACRT_CALL_FUNCTION_AND_RETURN_IF_AVAILABLE(IsValidLocaleName, lpLocaleName)
    // NOTE: On XP only installed locales are valid
    return IsValidLocale(__acrt_DownlevelLocaleNameToLCID(lpLocaleName), LCID_INSTALLED);
#endif // _CORECRT_WIN32_WINNT >= _WIN32_WINNT_VISTA
}

extern "C" int __cdecl __acrt_CompareStringEx(
    LPCWSTR const lpLocaleName,
    DWORD   const dwCmpFlags,
    LPCWSTR const lpString1,
    int     const cchCount1,
    LPCWSTR const lpString2,
    int     const cchCount2
    )
{
#if _CORECRT_WIN32_WINNT >= _WIN32_WINNT_VISTA
    return CompareStringEx(lpLocaleName, dwCmpFlags, lpString1, cchCount1, lpString2, cchCount2, nullptr, nullptr, 0);
#else // _CORECRT_WIN32_WINNT >= _WIN32_WINNT_VISTA
    _ACRT_CALL_FUNCTION_AND_RETURN_IF_AVAILABLE(CompareStringEx, lpLocaleName, dwCmpFlags, lpString1, cchCount1, lpString2, cchCount2, nullptr, nullptr, 0)
    return CompareStringW(__acrt_DownlevelLocaleNameToLCID(lpLocaleName), dwCmpFlags, lpString1, cchCount1, lpString2, cchCount2);
#endif // _CORECRT_WIN32_WINNT >= _WIN32_WINNT_VISTA
}


typedef BOOL (CALLBACK *PFNENUMLOCALESPROCEX)(LPWSTR, DWORD, LPARAM);

static PFNENUMLOCALESPROCEX __crtCallProc = nullptr;

extern "C" BOOL __cdecl __acrt_EnumSystemLocalesEx(
    LOCALE_ENUMPROCEX const lpLocaleEnumProcEx,
    DWORD             const dwFlags,
    LPARAM            const lParam
    )
{
#if _CORECRT_WIN32_WINNT >= _WIN32_WINNT_VISTA
    return EnumSystemLocalesEx(lpLocaleEnumProcEx, dwFlags, lParam, nullptr);
#else // _CORECRT_WIN32_WINNT >= _WIN32_WINNT_VISTA
    _ACRT_CALL_FUNCTION_AND_RETURN_IF_AVAILABLE(EnumSystemLocalesEx, lpLocaleEnumProcEx, dwFlags, lParam, nullptr)
    __crtCallProc = lpLocaleEnumProcEx;
    BOOL bRet = EnumSystemLocalesW([](LPWSTR lpLocaleString) { return __crtCallProc(lpLocaleString, 0, 0); }, LCID_INSTALLED);
    __crtCallProc = nullptr;
    return bRet;
#endif // _CORECRT_WIN32_WINNT >= _WIN32_WINNT_VISTA
}

extern "C" int __cdecl __acrt_GetDateFormatEx(
    LPCWSTR           const lpLocaleName,
    DWORD             const dwFlags,
    SYSTEMTIME const* const lpDate,
    LPCWSTR           const lpFormat,
    LPWSTR            const lpDateStr,
    int               const cchDate
    )
{
#if _CORECRT_WIN32_WINNT >= _WIN32_WINNT_VISTA
    return GetDateFormatEx(lpLocaleName, dwFlags, lpDate, lpFormat, lpDateStr, cchDate, nullptr);
#else // _CORECRT_WIN32_WINNT >= _WIN32_WINNT_VISTA
    _ACRT_CALL_FUNCTION_AND_RETURN_IF_AVAILABLE(GetDateFormatEx, lpLocaleName, dwFlags, lpDate, lpFormat, lpDateStr, cchDate, nullptr)
    return GetDateFormatW(__acrt_DownlevelLocaleNameToLCID(lpLocaleName), dwFlags, lpDate, lpFormat, lpDateStr, cchDate);
#endif // _CORECRT_WIN32_WINNT >= _WIN32_WINNT_VISTA
}

extern "C" int  __cdecl __acrt_GetTimeFormatEx(
    LPCWSTR           const lpLocaleName,
    DWORD             const dwFlags,
    SYSTEMTIME const* const lpTime,
    LPCWSTR           const lpFormat,
    LPWSTR            const lpTimeStr,
    int               const cchTime
    )
{
#if _CORECRT_WIN32_WINNT >= _WIN32_WINNT_VISTA
    return GetTimeFormatEx(lpLocaleName, dwFlags, lpTime, lpFormat, lpTimeStr, cchTime);
#else // _CORECRT_WIN32_WINNT >= _WIN32_WINNT_VISTA
    _ACRT_CALL_FUNCTION_AND_RETURN_IF_AVAILABLE(GetTimeFormatEx, lpLocaleName, dwFlags, lpTime, lpFormat, lpTimeStr, cchTime)
    return GetTimeFormatW(__acrt_DownlevelLocaleNameToLCID(lpLocaleName), dwFlags, lpTime, lpFormat, lpTimeStr, cchTime);
#endif // _CORECRT_WIN32_WINNT >= _WIN32_WINNT_VISTA    
}

extern "C" void __cdecl __acrt_GetSystemTimePreciseAsFileTime(
    LPFILETIME const lpSystemTimeAsFileTime
    )
{
#if _CORECRT_WIN32_WINNT >= _WIN32_WINNT_WIN8
    return GetSystemTimePreciseAsFileTime(lpSystemTimeAsFileTime);
#else // _CORECRT_WIN32_WINNT >= _WIN32_WINNT_WIN8
    _ACRT_CALL_FUNCTION_AND_RETURN_IF_AVAILABLE(GetSystemTimePreciseAsFileTime, lpSystemTimeAsFileTime)
    return GetSystemTimeAsFileTime(lpSystemTimeAsFileTime);
#endif // _CORECRT_WIN32_WINNT >= _WIN32_WINNT_WIN8    
}

extern "C" int  __cdecl __acrt_GetUserDefaultLocaleName(
    LPWSTR const lpLocaleName,
    int    const cchLocaleName
    )
{
#if _CORECRT_WIN32_WINNT >= _WIN32_WINNT_VISTA
    return GetUserDefaultLocaleName(lpLocaleName, cchLocaleName);
#else // _CORECRT_WIN32_WINNT >= _WIN32_WINNT_VISTA
    _ACRT_CALL_FUNCTION_AND_RETURN_IF_AVAILABLE(GetUserDefaultLocaleName, lpLocaleName, cchLocaleName)
    return __acrt_DownlevelLCIDToLocaleName(GetUserDefaultLCID(), lpLocaleName, cchLocaleName);
#endif // _CORECRT_WIN32_WINNT >= _WIN32_WINNT_VISTA    
}

extern "C" int __cdecl __acrt_LCMapStringEx(
    LPCWSTR const lpLocaleName,
    DWORD   const dwMapFlags,
    LPCWSTR const lpSrcStr,
    int     const cchSrc,
    LPWSTR  const lpDestStr,
    int     const cchDest
    )
{
#if _CORECRT_WIN32_WINNT >= _WIN32_WINNT_VISTA
    return LCMapStringEx(lpLocaleName, dwMapFlags, lpSrcStr, cchSrc, lpDestStr, cchDest, nullptr, nullptr, 0);
#else // _CORECRT_WIN32_WINNT >= _WIN32_WINNT_VISTA
    _ACRT_CALL_FUNCTION_AND_RETURN_IF_AVAILABLE(LCMapStringEx, lpLocaleName, dwMapFlags, lpSrcStr, cchSrc, lpDestStr, cchDest, nullptr, nullptr, 0)
    return LCMapStringW(__acrt_DownlevelLocaleNameToLCID(lpLocaleName), dwMapFlags, lpSrcStr, cchSrc, lpDestStr, cchDest);
#endif // _CORECRT_WIN32_WINNT >= _WIN32_WINNT_VISTA    
}

extern "C" BOOL __cdecl __acrt_GetFileInformationByHandleEx(
    HANDLE                    const hFile,
    FILE_INFO_BY_HANDLE_CLASS const FileInformationClass,
    LPVOID                    const lpFileInformation,
    DWORD                     const dwBufferSize
    )
{
#if _CORECRT_WIN32_WINNT >= _WIN32_WINNT_VISTA
    return GetFileInformationByHandleEx(hFile, FileInformationClass, lpFileInformation, dwBufferSize);
#else // _CORECRT_WIN32_WINNT >= _WIN32_WINNT_VISTA
    _ACRT_CALL_FUNCTION_AND_RETURN_IF_AVAILABLE(GetFileInformationByHandleEx, hFile, FileInformationClass, lpFileInformation, dwBufferSize)
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return 0;
#endif // _CORECRT_WIN32_WINNT >= _WIN32_WINNT_VISTA
}

#if _CORECRT_WIN32_WINNT < _WIN32_WINNT_VISTA
    extern "C" int __cdecl __acrt_LCIDToLocaleName(
        LCID   const lcid,
        LPWSTR const locale_name_result,
        int    const locale_name_count
        )
    {
        _ACRT_CALL_FUNCTION_AND_RETURN_IF_AVAILABLE(LCIDToLocaleName, lcid, locale_name_result, locale_name_count, 0)
        return __acrt_DownlevelLCIDToLocaleName(lcid, locale_name_result, locale_name_count);
    }
#endif // _CORECRT_WIN32_WINNT < _WIN32_WINNT_VISTA

extern "C" LCID __cdecl __acrt_LocaleNameToLCID(LPCWSTR const locale_name)
{
#if _CORECRT_WIN32_WINNT >= _WIN32_WINNT_VISTA
    return LocaleNameToLCID(locale_name, 0);
#else // _CORECRT_WIN32_WINNT >= _WIN32_WINNT_VISTA
    _ACRT_CALL_FUNCTION_AND_RETURN_IF_AVAILABLE(LocaleNameToLCID, locale_name, 0)
    return __acrt_DownlevelLocaleNameToLCID(locale_name);
#endif // _CORECRT_WIN32_WINNT >= _WIN32_WINNT_VISTA     
}

extern "C" HANDLE __cdecl __acrt_CreateFile2(
    LPCWSTR                           const lpFileName,
    DWORD                             const dwDesiredAccess,
    DWORD                             const dwShareMode,
    DWORD                             const dwCreationDisposition,
    LPCREATEFILE2_EXTENDED_PARAMETERS const pCreateExParams
    )
{
#if _CORECRT_WIN32_WINNT >= _WIN32_WINNT_WIN8
    if (!__acrt_has_desktop_support())
    {
        return CreateFile2(lpFileName, dwDesiredAccess, dwShareMode, dwCreationDisposition, pCreateExParams);
    }
#endif

    return __acrt_CreateFileW(
        lpFileName,
        dwDesiredAccess,
        dwShareMode,
        pCreateExParams->lpSecurityAttributes,
        dwCreationDisposition,
        pCreateExParams->dwFileAttributes | pCreateExParams->dwFileFlags,
        nullptr);
}

extern "C" BOOL __cdecl __acrt_IsPackagedApp()
{
    return FALSE;
}


#ifdef _M_X64

extern "C" void __cdecl __acrt_CaptureCurrentContext(CONTEXT* const pContextRecord)
{
    ULONG64 ControlPc;
    ULONG64 EstablisherFrame;
    ULONG64 ImageBase;
    PRUNTIME_FUNCTION FunctionEntry;
    PVOID HandlerData;

    RtlCaptureContext(pContextRecord);

    ControlPc = pContextRecord->Rip;
    FunctionEntry = RtlLookupFunctionEntry(ControlPc, &ImageBase, nullptr);

    if (FunctionEntry != nullptr)
    {
        RtlVirtualUnwind(
            UNW_FLAG_NHANDLER,
            ImageBase,
            ControlPc,
            FunctionEntry,
            pContextRecord,
            &HandlerData,
            &EstablisherFrame,
            nullptr);
    }
}

extern "C" void __cdecl __acrt_CapturePreviousContext(CONTEXT* const pContextRecord)
{
    ULONG64 ControlPc;
    ULONG64 EstablisherFrame;
    ULONG64 ImageBase;
    PRUNTIME_FUNCTION FunctionEntry;
    PVOID HandlerData;
    INT frames;

    RtlCaptureContext(pContextRecord);

    ControlPc = pContextRecord->Rip;

    /* Unwind "twice" to get the context of the caller to the "previous" caller. */
    for (frames = 0; frames < 2; ++frames)
    {
        FunctionEntry = RtlLookupFunctionEntry(ControlPc, &ImageBase, nullptr);

        if (FunctionEntry != nullptr)
        {
            RtlVirtualUnwind(
                UNW_FLAG_NHANDLER,
                ImageBase,
                ControlPc,
                FunctionEntry,
                pContextRecord,
                &HandlerData,
                &EstablisherFrame,
                nullptr);
        }
        else
        {
            break;
        }
    }
}

#endif // _M_X64


// Terminates the current process.  This function is not called on ARM or
// Windows 8.  On those platforms, __fastfail() is used instead.
extern "C" void __cdecl __acrt_TerminateProcess(UINT const exit_code)
{
    // Terminate the current process.  The return code is currently unusable in
    // the CRT, so we ignore it.
    TerminateProcess(GetCurrentProcess(), exit_code);
}


// Raise an exception that will override all other exception filters.  In most
// cases, if the process is not being debugged, the function displays an error
// message box (Watson box).
//
// Returns EXCEPTION_CONTINUE_SEARCH or EXCEPTION_EXECUTE_HANDLER if
// SEM_NOGPFAULTERRORBOX flag was specified in a previous call to SetErrorMode.
extern "C" LONG __cdecl __acrt_UnhandledException(EXCEPTION_POINTERS* const exception_info_)
{
    // specifies default handling within UnhandledExceptionFilter
    SetUnhandledExceptionFilter(nullptr);

    // invoke default exception filter
    return UnhandledExceptionFilter(exception_info_);
}
