#include <fenv.h>
#include "_fenvutils.h"


int __cdecl fegetenv(fenv_t *env) {
	env->_Fe_ctl = _getfpcontrolword();
	env->_Fe_stat = _getfpstatusword();
	return 0;
}



