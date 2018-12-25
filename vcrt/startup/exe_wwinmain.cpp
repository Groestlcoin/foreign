//
// exe_wwinmain.cpp
//
//      Copyright (c) Microsoft Corporation. All rights reserved.
//
// The wWinMainCRTStartup() entry point, linked into client executables that
// uses wWinMain().
//
#define _SCRT_STARTUP_WWINMAIN
#define CRT_PFX exe_wwinmain
#include "exe_common.inl"



extern "C" int wWinMainCRTStartup()
{
    return __scrt_common_main();
}
