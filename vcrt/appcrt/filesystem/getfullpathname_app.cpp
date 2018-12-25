/***
*getfullpathname_app.cpp - getting fullpath name
*
*    Copyright (c) Microsoft Corporation. All rights reserved.
*
*Purpose:
*    defines __crtGetFullPathNameWinRTW()/__crtGetFullPathNameWinRTA() for WINAPI_FAMILY_APP - getting fullpath name
*
*******************************************************************************/

#include <corecrt_internal.h>

#if _CORECRT_WIN32_WINNT >= _WIN32_WINNT_WIN8

#include <wrl\wrappers\corewrappers.h>
#include <wrl\client.h>
#include <windows.applicationmodel.h>
#include <windows.storage.h>
#include <wchar.h>

INIT_ONCE currentDirDataInitOnce = INIT_ONCE_STATIC_INIT;
wchar_t currentDir[_MAX_DIR] = { 0 };
wchar_t currentDrive[_MAX_DRIVE] = { 0 };

static BOOL CALLBACK initCurrentDirData(
    PINIT_ONCE const init_once,
    PVOID      const parameter,
    PVOID*     const context
    )
{
    UNREFERENCED_PARAMETER(init_once);
    UNREFERENCED_PARAMETER(parameter);
    UNREFERENCED_PARAMETER(context);

    ::Microsoft::WRL::ComPtr< ::ABI::Windows::ApplicationModel::IPackageStatics> applicationPackage;
    ::Microsoft::WRL::ComPtr< ::ABI::Windows::ApplicationModel::IPackage> package;
    ::Microsoft::WRL::ComPtr< ::ABI::Windows::Storage::IStorageFolder> storageFolder;
    ::Microsoft::WRL::ComPtr< ::ABI::Windows::Storage::IStorageItem> storageItem;
    wchar_t currentPath[MAX_PATH] = { 0 };

    HRESULT hr = ::Windows::Foundation::GetActivationFactory(
        ::Microsoft::WRL::Wrappers::HStringReference(RuntimeClass_Windows_ApplicationModel_Package).Get(),
        applicationPackage.GetAddressOf());
    if (FAILED(hr))
    {
        return FALSE;
    }

    hr = applicationPackage->get_Current(package.GetAddressOf());
    if (FAILED(hr))
    {
        return FALSE;
    }

    hr = package->get_InstalledLocation(storageFolder.GetAddressOf());
    if (FAILED(hr))
    {
        return FALSE;
    }

    hr = storageFolder.As(&storageItem);
    if (FAILED(hr))
    {
        return FALSE;
    }

    ::Microsoft::WRL::Wrappers::HString installedLocation;
    hr = storageItem->get_Path(installedLocation.GetAddressOf());
    if (FAILED(hr))
    {
        return FALSE;
    }

    unsigned int length;
    auto buffer = installedLocation.GetRawBuffer(&length);

    if (length >= MAX_PATH - 1)
    {
        return 0;
    }

    wmemcpy(currentPath, buffer, length);
    currentPath[length] = L'\\'; //Make sure that there is \ at the end of path

    if (_wsplitpath_s(currentPath, currentDrive, _countof(currentDrive), currentDir, _countof(currentDir), nullptr, 0, nullptr, 0))
    {
        return FALSE;
    }

    return TRUE;
}


/***
*int __crtGetFullPathNameWinRTW(const wchar_t* fileName, DWORD bufferLength, wchar_t* buffer) - returns string the absolute path
*
*Purpose:
*       returns string with the absolute path
*
*Entry:
*   wchar_t* - file name
*   DWORD - buffer length
*   wchar_t* - buffer
*
*Exit:
*       return path string length or 0 if failed
*
*Exceptions:
*
*******************************************************************************/
extern "C" DWORD __cdecl __crtGetFullPathNameWinRTW(const wchar_t* fileName, DWORD bufferLength, wchar_t* buffer)
{
    if (!InitOnceExecuteOnce(&currentDirDataInitOnce, initCurrentDirData, nullptr, nullptr))
    {
        return 0;
    }

    wchar_t fullPath[MAX_PATH] = { 0 };
    wchar_t* result = fullPath;
    wchar_t drive[_MAX_DRIVE] = { 0 };
    wchar_t dir[_MAX_DIR] = { 0 };
    wchar_t fname[_MAX_FNAME] = { 0 };
    wchar_t ext[_MAX_EXT] = { 0 };

    size_t length = wcslen(fileName);

    if (fileName == nullptr || length >= MAX_PATH)
    {
        return 0;
    }

    for (size_t i = 0; i < length; i++)
    {
        if (fileName[i] == '/')
        {
            fullPath[i] = '\\';
        }
        else
        {
            fullPath[i] = fileName[i];
        }
    }

    if (_wsplitpath_s(fullPath, drive, _countof(drive), dir, _countof(dir), fname, _countof(fname), ext, _countof(ext)))
    {
        return 0;
    }

    if (length >= 2 && wcsncmp(fullPath, L"\\\\", 2) == 0) {}
    else if (length >= 1 && fullPath[0] == '\\')
    {
        if (_wmakepath_s(fullPath, _countof(fullPath), currentDrive, dir, fname, ext))
        {
            return 0;
        }
    }
    else if (drive[0] == '\0')
    {
        wchar_t combineDir[MAX_PATH] = { 0 };

        _ERRCHECK(wcscat_s(combineDir, MAX_PATH, currentDir));
        _ERRCHECK(wcscat_s(combineDir, MAX_PATH, dir));

        if (_wmakepath_s(fullPath, _countof(fullPath), currentDrive, combineDir, fname, ext))
        {
            return 0;
        }
    }

    if (bufferLength != 0 && buffer != nullptr)
    {
        ::wmemcpy(buffer, result, bufferLength);
    }

    return static_cast<DWORD>(wcslen(result));
}

/***
*int __crtGetFullPathNameWinRTA(const char* fileName, DWORD bufferLength, char* buffer) - returns string the absolute path
*
*Purpose:
*       returns string with the absolute path
*
*Entry:
*   wchar_t* - file name
*   DWORD - buffer length
*   wchar_t* - buffer
*
*Exit:
*       return path string length or 0 if failed
*
*Exceptions:
*
*******************************************************************************/
extern "C" DWORD __cdecl __crtGetFullPathNameWinRTA(const char* fileName, DWORD bufferLength, char* buffer)
{
    wchar_t *wszFileName;
    wchar_t fullPath[MAX_PATH] = { 0 };

    if (!__acrt_copy_path_to_wide_string(fileName, &wszFileName))
    {
        return 0;
    }

    DWORD count = __crtGetFullPathNameWinRTW(wszFileName, MAX_PATH, fullPath);
    _free_crt(wszFileName);

    if (bufferLength != 0 && buffer != nullptr)
    {
        char *ansiPath;
        if (!__acrt_copy_to_char(fullPath, &ansiPath))
        {
            return 0;
        }

        if (::strlen(ansiPath) >= bufferLength)
        {
            _free_crt(ansiPath);
            return 0;
        }

        memcpy(buffer, ansiPath, bufferLength);
        _free_crt(ansiPath);
    }

    return count;
}

#endif // _CORECRT_WIN32_WINNT >= _WIN32_WINNT_WIN8
