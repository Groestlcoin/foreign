#define WIN32
#define _WINDOWS
#define UCFG_WIN_HEADERS 1

#ifdef _MSC_VER
#	include <vc-inc.h>
#endif

#pragma warning (disable: 4101 4242 4244 4350 4458 4789)

#define HAVE_GETTIMEOFDAY 1
#define HAVE_ISALPHA 1
#define HAVE_UNIX98_PRINTF 1
#define HAVE_SMALLBUILD 1

#undef getpid
#define getpid() ((int)GetCurrentProcessId())

#ifndef _DLL
#	define _LIB
#endif
