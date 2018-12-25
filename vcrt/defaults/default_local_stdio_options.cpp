#include <vcstartup_internal.h>
#include <corecrt_stdio_config.h>

extern "C" void __cdecl __scrt_initialize_default_local_stdio_options() {
	_CRT_INTERNAL_LOCAL_PRINTF_OPTIONS |= _CRT_INTERNAL_PRINTF_LEGACY_WIDE_SPECIFIERS;
	_CRT_INTERNAL_LOCAL_SCANF_OPTIONS |= _CRT_INTERNAL_SCANF_LEGACY_WIDE_SPECIFIERS;
}