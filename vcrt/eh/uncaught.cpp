#include <eh.h>
#include <vcruntime_internal.h>

extern "C" int __cdecl __uncaught_exceptions()
{
    return __vcrt_getptd()->_ProcessingThrow;
}
