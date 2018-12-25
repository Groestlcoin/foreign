#include <fenv.h>
#include "_fenvutils.h"

int __cdecl fegetexceptflag(fexcept_t *flagp, int excepts) {
	*flagp = _getfpstatusword() & excepts;
	return 0;
}
