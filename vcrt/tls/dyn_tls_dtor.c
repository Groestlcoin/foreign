#include <vcruntime_internal.h>
#include <vcstartup_internal.h>

extern "C" {

extern const _tls_callback_type __dyn_tls_dtor_callback;

_tls_callback_type const* __cdecl __scrt_get_dyn_tls_dtor_callback() {
	return &__dyn_tls_dtor_callback;

}

}
