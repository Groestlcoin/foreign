/***
*hooks.cpp - global (per-thread) variables and functions for EH callbacks
*
*       Copyright (c) Microsoft Corporation. All rights reserved.
*
*Purpose:
*       global (per-thread) variables for assorted callbacks, and
*       the functions that do those callbacks.
*
*       Entry Points:
*
*       * terminate()
*       * unexpected()
*
****/

#include <eh.h>
#include <ehassert.h>
#include <ehhooks.h>  
#include <excpt.h>
#include <stdlib.h>


/////////////////////////////////////////////////////////////////////////////
//
// terminate - call the terminate handler (presumably we went south).
//              THIS MUST NEVER RETURN!
//
// Open issues:
//      * How do we guarantee that the whole process has stopped, and not just
//        the current thread?
//

__declspec(noreturn) void __cdecl terminate(void)
{
        EHTRACE_ENTER_MSG("No exit");

        {
            terminate_function pTerminate;
            pTerminate = __pTerminate;
            if ( pTerminate != nullptr ) {
                /*
                Note: If the user's terminate handler crashes, we cannot allow an EH to propagate
                as we may be in the middle of exception processing, so we abort instead.
                */
                __try {
                    //
                    // Let the user wrap things up their way.
                    //
                    pTerminate();
                }
                __except (EXCEPTION_EXECUTE_HANDLER) {
                    //
                    // Intercept ANY exception from the terminate handler
                    //
                    ; // Deliberately do nothing
                }
            }
        }

        //
        // If the terminate handler returned, faulted, or otherwise failed to
        // halt the process/thread, we'll do it.
        //
        abort();
}

/////////////////////////////////////////////////////////////////////////////
//
// unexpected - call the unexpected handler (presumably we went south, or nearly).
//              THIS MUST NEVER RETURN!
//
// Open issues:
//      * How do we guarantee that the whole process has stopped, and not just
//        the current thread?
//

void __cdecl unexpected(void)
{
        unexpected_function pUnexpected;

        EHTRACE_ENTER;

        pUnexpected = __pUnexpected;
        if ( pUnexpected != nullptr ) {
        //
        // Let the user wrap things up their way.
        //
            pUnexpected();
        }

        //
        // If the unexpected handler returned, we'll give the terminate handler a chance.
        //
        terminate();
}
