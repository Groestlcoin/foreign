#include <el/libext.h>

#define BUILD_WINDOWS

#include <windows.h>

#pragma warning(disable: 4018 4055 4100 4127 4132 4146 4232 4242 4244 4389 4456 4701 4703 4706)

#define SQLITE_THREADSAFE 2
#define SQLITE_ENABLE_COLUMN_METADATA 1
#define SQLITE_ENABLE_RTREE 1
#define SQLITE_OMIT_LOCALTIME 1
#define SQLITE_DEFAULT_FOREIGN_KEYS 1

#ifdef _DEBUG
#	define SQLITE_DEBUG 1

#	if UCFG_WCE
//#		define SQLITE_NO_SYNC 1			// bug in the CE 5.0 Emulator's FlushFileBuffers for \Storage Card
#	endif
#endif