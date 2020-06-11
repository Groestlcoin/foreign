#define UCFG_WIN_HEADERS 1

#ifdef _MSC_VER
#	include <vc-inc.h>
#endif

#include <winsock2.h>

#define OPENSSL_BUILD_SHLIBCRYPTO
#define OPENSSL_NO_CAMELLIA
#define OPENSSL_SMALL_FOOTPRINT
#define OPENSSL_NO_AFALGENG
#define OPENSSL_NO_CRYPTO_MDEBUG_BACKTRACE
#define OPENSSL_NO_CT
#define OPENSSL_NO_DEVCRYPTOENG
#define OPENSSL_NO_SEED

#pragma warning(disable: 4005 4057 4090 4130 4132 4133 4152 4232 4242 4244 4245 4295 4305 4306 4310 4311 4505 4701)



#if defined(_MSC_VER) && (defined(_M_IX86) || defined(_M_X64)) && defined(UCFG_OPENSSL_ASM) && UCFG_OPENSSL_ASM
#	define UCFG_BN_ASM 1
#	ifdef UCFG_LIBEXT
//	 #define OPENSSL_NO_ASM
//!!!T #		define OPENSSL_BN_ASM_MONT
#		include <el/bignum.h>
#	else
	__BEGIN_DECLS
		typedef uintptr_t BASEWORD;
		BASEWORD __cdecl ImpMulAddBignums(BASEWORD *r, const BASEWORD *a, size_t n, BASEWORD w);
	__END_DECLS
#	endif
#	define bn_mul_add_words ImpMulAddBignums
#else
#	define UCFG_BN_ASM 0
#endif

const char *gai_strerror(int code);

#if UCFG_WCE
#	define OPENSSL_SYS_WINCE 1
#	define OPENSSL_NO_HW_AEP 1
#	define OPENSSL_NO_CAPIENG 1
#	define OPENSSL_NO_ERR 1
#	define OPENSSL_NO_SCTP 1
#	define OPENSSL_NO_MDC2 1
//#	define OPENSSL_NO_FP_API 1
//#	define OPENSSL_NO_STDIO 1
#endif
#define OPENSSL_THREADS 1

#define OPENSSL_RAND_SEED_OS

#define DllMain C_DllMain

#ifdef OPENSSL_NO_OBJECT
#	error Objects required to export PrivateKeys
#endif

#define OPENSSL_NO_KTLS

#include "opensslconf.h"
