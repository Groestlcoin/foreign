#include <vcruntime_internal.h>
#include <stdint.h>

int __favor = __FAVOR_ENFSTRG;

int __isa_enabled = 1;
int __isa_available = __ISA_AVAILABLE_SSE2;


uint64_t __memcpy_nt_iters = 8239;

int __cdecl __isa_available_init(void) {
	return 0;
}
