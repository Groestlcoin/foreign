#include <fenv.h>
#include "_fenvutils.h"


int __cdecl fesetenv(const fenv_t *env) {
	_setfpcontrolword(env->_Fe_ctl);
	_setfpstatusword(env->_Fe_stat);
	fenv_t n;
	fegetenv(&n);
	return n._Fe_ctl != env->_Fe_ctl || n._Fe_stat != env->_Fe_stat;
}



