#define _RTC
#include <rtcapi.h>
#include <crtdbg.h>

static _RTC_error_fn _RTC_ErrorReportFunc;
static _RTC_error_fnW _RTC_ErrorReportFuncW;

_RTC_error_fnW __cdecl _RTC_SetErrorFuncW(_RTC_error_fnW pfn) {
	_RTC_error_fnW r = _RTC_ErrorReportFuncW;
	_RTC_ErrorReportFuncW = pfn;
	_RTC_ErrorReportFunc = 0;
	return r;
}
