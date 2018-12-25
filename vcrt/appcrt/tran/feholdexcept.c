#include <fenv.h>
#include "_fenvutils.h"


int __cdecl feholdexcept(fenv_t *env) {
	fenv_t tenv;
	if (fegetenv(&tenv))
		return 1;
	*env = tenv;
	tenv._Fe_ctl |= FE_ALL_EXCEPT;
	if (fesetenv(&tenv))
		return 1;
	_clearfp();
	return 0;
}



