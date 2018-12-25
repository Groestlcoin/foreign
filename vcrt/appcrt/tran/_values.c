#include <math.h>
#include <float.h>

const _float_const
	_Denorm_C	= { 1, 0, 0, 0 },
	_Inf_C		= { 0, 0, 0, 0x7FF0 },
	_Nan_C		= { 0, 0, 0, 0x7FF8 },
	_Snan_C		= { 1, 0, 0, 0x7FF0 },
	_Hugeval_C	= { 0, 0, 0, 0x7FF0 },
	_FDenorm_C	= { 1, 0 },
	_FInf_C		= { 0, 0x7F80 },
	_FNan_C		= { 0, 0x7FC0 },
	_FSnan_C	= { 1, 0x7F80 },
	_LDenorm_C	= { 1, 0, 0, 0 },
	_LInf_C		= { 0, 0, 0, 0x7FF0 },
	_LNan_C		= { 0, 0, 0, 0x7FF8 },
	_LSnan_C	= { 1, 0, 0, 0x7FF0 },
	_Eps_C		= { 0, 0, 0, 0x3C90 },
	_Rteps_C	= { 0, 0, 0, 0x3E40 },
	_FEps_C		= { 0, 0x3300 },
	_FRteps_C	= { 0, 0x3980 },
	_LEps_C		= { 0, 0, 0, 0x3C90 },
	_LRteps_C	= { 0, 0, 0, 0x3E40 };

const double
	_Zero_C = 0,
	_Xbig_C = 18;

const float
	_FZero_C = 0,
	_FXbig_C = 8;

const long double
	_LZero_C = 0,
	_LXbig_C = 18;



