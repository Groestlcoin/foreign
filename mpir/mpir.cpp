#include <mpir.h>

#ifdef UCFG_LIBEXT
using namespace Ext;

static void *GmpMallocFun(size_t size) {
	return Malloc(size);
}

static void *GmpRealloc(void *p, size_t oldSize, size_t newSize) {
	return Realloc(p, newSize);
}

static void GmpFree(void *p, size_t size) {
	free(p);
}

class GmpMalloc {
public:
	GmpMalloc() {
		::mp_set_memory_functions(&GmpMallocFun, &GmpRealloc, &GmpFree);
	}
};


#if UCFG_ALLOCATOR != 'S'
	static GmpMalloc s_gmpMalloc;
#endif


#endif // UCFG_LIBEXT


