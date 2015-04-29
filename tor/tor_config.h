#define WIN32
#define UCFG_DEFINE_NDEBUG 0

#define UCFG_WIN_HEADERS 1

#ifndef __cplusplus
#	define _CRT_TERMINATE_DEFINED
	void __cdecl MyExit(int c);
	void __cdecl MyAbort(void);
#	define exit C_Exit
#	define _exit C__Exit
#	define abort C_Abort
#endif


#include <el/libext.h>

#ifndef __cplusplus
#	undef exit
#	undef _exit
#	undef abort
#	define exit MyExit
#	define _exit MyExit
#	define abort MyAbort
#endif


#if NTDDI_VERSION < 0x6000000
#error
#endif


#define HAVE_STRUCT_IN6_ADDR 1
#define HAVE_STRUCT_SOCKADDR_IN6 1

#define HAVE_GETNAMEINFO 1
#define HAVE_GETADDRINFO 1

//#define HAVE_SYS_TIME_H 1


#include <WS2tcpip.h>


#define EAI_SYSTEM       -11 //!!! was WSAEINVAL

#pragma warning (disable: 4005 4028 4054 4057 4090 4101 4130 4152 4200 4204 4242 4244 4245 4295 4305 4334 4459 4668 4701 4703)
#define SHARE_DATADIR "."

#define _EVENT_HAVE_SA_FAMILY_T
#define _EVENT_HAVE_STRSEP 1
#define _EVENT_HAVE_STRLCPY 1


//#define TOR_DI_OPS_H
//!!!R #define tor_memcmp memcmp
//!!!? #define tor_memeq(a, b, c) (!memcmp(a, b, c))
//!!!? #define tor_memneq(a,b,sz) (!tor_memeq((a),(b),(sz)))

#define fast_memcmp(a,b,c) (memcmp((a),(b),(c)))
#define fast_memeq(a,b,c)  (0==memcmp((a),(b),(c)))
#define fast_memneq(a,b,c) (0!=memcmp((a),(b),(c)))

#define inline __inline



//!!! tor don't work normally with libevent2

//#define HAVE_EVENT2_EVENT_H 1
//#define HAVE_EVENT2_DNS_H 1

#if UCFG_CRT!='U'
#	define open API_open
#endif

#define HAVE_EXTERN_ENVIRON_DECLARED

