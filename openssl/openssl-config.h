#define UCFG_WIN_HEADERS 1

#include <el/libext.h>

#include <winsock2.h>

#define OPENSSL_BUILD_SHLIBCRYPTO
#define OPENSSL_NO_SEED
#define OPENSSL_NO_CAMELLIA
#define OPENSSL_SMALL_FOOTPRINT

#pragma warning(disable: 4057 4132 4152 4232 4242 4244 4245 4295 4305 4306 4311 4505 4701)

#if defined(_M_X64)
#	define OPENSSL_NO_ASM
#	define OPENSSL_NO_HW_AEP

#	define CONFIG_HEADER_BN_H
#	define SIXTY_FOUR_BIT
#endif

#if defined(_M_IX86) || defined(_M_X64)
//	 #define OPENSSL_NO_ASM
#	define OPENSSL_BN_ASM_MONT
#	define UCFG_BN_ASM 1
#	include <el/bignum.h>

#	define bn_mul_add_words ImpMulAddBignums
#else
#	define UCFG_BN_ASM 0
#endif

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


#define DllMain C_DllMain

#ifdef OPENSSL_NO_OBJECT
#	error Objects required to export PrivateKeys
#endif

#include "opensslconf.h"

