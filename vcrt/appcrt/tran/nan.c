#include <math.h>

int __cdecl _dsign(double x) {
	return 0x80 & ((char*)&x)[7];
}

double __cdecl nan(const char *tagp) {
	return _Nan_C._Double;
}
