#include <el/libext.h>
#pragma warning(disable: 4101 4102 4146 4242 4244 4245 4293 4307 4310 4334 4505 4701 4723)

#define WANT_TMP_ALLOCA 1

#define HAVE_STDARG 1

#undef DEBUG 	// defined(DEBUG) does not allow some pointer hack optimizations

#if defined(_M_IX86) || defined(_M_X64)
//#	define __gmpn_addmul_1 ImpMulAddBignums
#endif
