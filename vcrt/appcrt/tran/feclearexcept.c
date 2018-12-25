#include <fenv.h>
#include "_fenvutils.h"

int __cdecl feclearexcept(int excepts) {
	uint32_t st = _getfpstatusword();
	if (st & excepts) {
		int newExcepts = st & ~excepts;
		_setfpstatusword(newExcepts);
		return fetestexcept(newExcepts) != newExcepts;
	}
	return 0;
}
