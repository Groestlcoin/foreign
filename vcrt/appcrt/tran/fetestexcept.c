#include <fenv.h>

int __cdecl fetestexcept(int excepts) {
	fexcept_t flag = 0;
	fegetexceptflag(&flag, excepts);
	return flag;
}
