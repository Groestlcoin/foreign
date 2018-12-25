#pragma once

#define DISABLE_SHRINK_WRAPPING()

#define ENABLE_SHRINK_WRAPPING()

_CRT_BEGIN_C_HEADER


typedef struct _GS_HANDLER_DATA *PGS_HANDLER_DATA;

void
__GSHandlerCheckCommon (
    IN PVOID EstablisherFrame,
    IN PDISPATCHER_CONTEXT DispatcherContext,
    IN PGS_HANDLER_DATA GSHandlerData
    );

_CRT_END_C_HEADER
