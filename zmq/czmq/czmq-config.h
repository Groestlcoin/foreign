#pragma once

#ifdef _MSC_VER
#	include <vc-inc.h>
#endif

#pragma warning(disable: 4028 4142 4242 4244)

#define ZMQ_USE_SELECT

#ifndef _DLL
#	define ZMQ_STATIC
#	define LIBCZMQ_STATIC
#	define LIBCZMQPP_STATIC
#endif

