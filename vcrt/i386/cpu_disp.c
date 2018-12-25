#include <vcruntime_internal.h>

int __favor = 0;
int __isa_enabled = 1;
int __isa_available = __ISA_AVAILABLE_X86;

int __cdecl __isa_available_init(void) {
	if (IsProcessorFeaturePresent(PF_XMMI64_INSTRUCTIONS_AVAILABLE)) {
		__isa_available = __ISA_AVAILABLE_SSE2;
		int ar[4];
		__cpuid(ar, 1);
		if (ar[2] & 0x100000)  // FMA
			__isa_available = (ar[2] & 0x10000000) ? __ISA_AVAILABLE_AVX : __ISA_AVAILABLE_SSE42;
	}
	return 0;
}

_CRTALLOC(".CRT$XIC") _PIFV p_isa_available_init = __isa_available_init;

