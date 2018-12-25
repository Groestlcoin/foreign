#include <winternl.h>

#include <vcwininternls.h>

#define EH_EXCEPTION_NUMBER ('msc' | 0xE0000000)

void __cdecl terminate();

EXCEPTION_DISPOSITION
__C_specific_handler_noexcept (
    _In_ PEXCEPTION_RECORD ExceptionRecord,
    _In_ PVOID EstablisherFrame,
    _Inout_ PCONTEXT ContextRecord,
    _Inout_ PDISPATCHER_CONTEXT DispatcherContext
    )
{
	EXCEPTION_DISPOSITION r = __C_specific_handler(ExceptionRecord, EstablisherFrame, ContextRecord, DispatcherContext);
	if (IS_DISPATCHING(ExceptionRecord->ExceptionFlags)
			&& ExceptionRecord->ExceptionCode == EH_EXCEPTION_NUMBER
			&& r == ExceptionContinueSearch) {
		terminate();
	}

	return r;
}
