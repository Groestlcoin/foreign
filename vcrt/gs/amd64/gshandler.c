/***
*gshandlereh.c - Defines __GSHandlerCheck_EH for X64
*
*       Copyright (c) Microsoft Corporation.  All rights reserved.
*
*Purpose:
*       Defines __GSHandlerCheck_EH, the X64 exception handler for functions
*       with /GS-protected buffers as well as C++ EH constructs.
*******************************************************************************/

#include <winternl.h>
#include <vcwininternls.h>


void
__GSHandlerCheckCommon (
    IN PVOID EstablisherFrame,
    IN PDISPATCHER_CONTEXT DispatcherContext,
    IN PGS_HANDLER_DATA GSHandlerData
    )
{
 //!!!TODO
}

int
__GSHandlerCheck (void *res1,
    IN PVOID EstablisherFrame,
    void *res2,
    IN PDISPATCHER_CONTEXT DispatcherContext
    )
{
	__GSHandlerCheckCommon(EstablisherFrame, DispatcherContext, DispatcherContext->HandlerData);
	return 1;
}

