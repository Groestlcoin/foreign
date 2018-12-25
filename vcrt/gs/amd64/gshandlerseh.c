#include <winternl.h>
#include <vcwininternls.h>

EXCEPTION_DISPOSITION
__GSHandlerCheck_SEH (
    IN PEXCEPTION_RECORD ExceptionRecord,
    IN PVOID EstablisherFrame,
    IN OUT PCONTEXT ContextRecord,
    IN OUT PDISPATCHER_CONTEXT DispatcherContext
    )
{
	//!!!TODO
    PGS_HANDLER_DATA GSHandlerData = (PGS_HANDLER_DATA)((PULONG)DispatcherContext->HandlerData + 1);

    //
    // Perform the actual cookie check.
    //

    __GSHandlerCheckCommon(
        EstablisherFrame,
        DispatcherContext,
        GSHandlerData
        );

	return __C_specific_handler(ExceptionRecord, EstablisherFrame, ContextRecord, DispatcherContext);
}
