#include <fenv.h>
#include <float.h>
#include <math.h>


short __cdecl _dtest(double* _Px) {
	const unsigned short *w = (unsigned short*)_Px;
	return (w[3] & 0x7FF0) == 0x7FF0 ? ((w[3] & 0xF) || w[0] || w[1] || w[2] ? FP_NAN : FP_INFINITE)
		: !(w[3] & 0x7FFF) && !w[0] && !w[1] && !w[2] ? FP_ZERO
		: w[3] & 0x7FF0 ? FP_NORMAL
		: FP_SUBNORMAL;
}

short __cdecl _dclass(double _X) {
	return _dtest(&_X);
}