#include <el/ext.h>

#include <windows.h>

//#include <el/stl/string.h>
//#include EXT_HEADER_SYSTEM_ERROR

extern "C" {
	errno_t __cdecl rand_s(unsigned int* randval);
}

namespace Ext {
	typedef void (AFXAPI *PFNThrowImp)(HRESULT hr);
	PFNThrowImp g_pfnThrowImp;
	void SetThrowImp(PFNThrowImp pfn) { g_pfnThrowImp = pfn; }
} // Ext::

namespace std {

__declspec(noreturn) void __cdecl _Xinvalid_argument(const char *msg) {
	Ext::g_pfnThrowImp(E_INVALIDARG); //!!!T	throw invalid_argument(msg);
}

void __cdecl _Rng_abort(const char *msg) {
	fputs(msg, stderr);
	fputc('\n', stderr);
	abort();
}

unsigned int __cdecl _Random_device() {	// return a random value
	unsigned int ans;
	if (rand_s(&ans))
		Ext::g_pfnThrowImp(MAKE_HRESULT(SEVERITY_ERROR, FACILITY_EXT, 2));	// ExtErr::IndexOutOfRange
	return ans;
}

} // std::
