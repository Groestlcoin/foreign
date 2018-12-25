#pragma once

#include <stdint.h>


_CRT_BEGIN_C_HEADER

uint32_t __cdecl _getfpcontrolword(void);
uint32_t __cdecl _getfpstatusword(void);
void __cdecl _setfpcontrolword(uint32_t v);
void __cdecl _setfpstatusword(uint32_t v);

_CRT_END_C_HEADER
