#include <winternl.h>

#define EH_EXCEPTION_NUMBER ('msc' | 0xE0000000)

EXCEPTION_DISPOSITION __cdecl _except_handler3(
	_In_ PEXCEPTION_RECORD ExceptionRecord,
	_In_ PVOID EstablisherFrame,
	_Inout_ PCONTEXT ContextRecord,
	_Inout_ VOID *dispatcherContext
);
void __cdecl terminate();

EXCEPTION_DISPOSITION
_except_handler3_noexcept (
    _In_ PEXCEPTION_RECORD ExceptionRecord,
    _In_ PVOID EstablisherFrame,
    _Inout_ PCONTEXT ContextRecord,
    _Inout_ VOID *dispatcherContext
    )
{
	EXCEPTION_DISPOSITION r = _except_handler3(ExceptionRecord, EstablisherFrame, ContextRecord, dispatcherContext);
	if (IS_DISPATCHING(ExceptionRecord->ExceptionFlags)
			&& ExceptionRecord->ExceptionCode == EH_EXCEPTION_NUMBER
			&& r == ExceptionContinueSearch) {
		terminate();
	}
	return r;
}
