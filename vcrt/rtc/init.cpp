#define _RTC
#include <rtcapi.h>
#include <crtdbg.h>


_RTC_error_fnW _CRT_RTC_INITW() {
#ifdef _DEBUG
	return &_CrtDbgReportW;
#else
	return 0;
#endif
}

extern "C" void __cdecl _RTC_InitBase() {
	static bool init;
	if (!init) {
		init = true;
		_RTC_SetErrorFuncW(_CRT_RTC_INITW());
	}

}

extern "C" void __cdecl _RTC_Shutdown() {
	_CRT_RTC_INITW();
}


