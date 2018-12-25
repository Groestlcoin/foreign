#include <fenv.h>
#include "_fenvutils.h"

int __cdecl fegetround(void) {
	return _getfpcontrolword() & FE_ROUND_MASK;
}

