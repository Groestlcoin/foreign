#ifndef _MSC_VER
#	define EXT_GMP_HEADER <gmp.h>
#	define EXT_GMP_LIB "gmp"
#else
#	define EXT_GMP_HEADER <mpir.h>
#	define EXT_GMP_LIB "mpir"
#endif

#define USE_NUM_GMP
#define USE_FIELD_GMP
#define USE_FIELD_INV_BUILTIN

