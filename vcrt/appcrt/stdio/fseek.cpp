//
// fseek.cpp
//
//      Copyright (c) Microsoft Corporation.  All rights reserved.
//
// Defines the fseek() family of functions, which repositions a file pointer to
// a new place in a stream.
//
#include <corecrt_internal_stdio.h>



static int __cdecl common_fseek_nolock(__crt_stdio_stream const stream, __int64 offset, int whence) throw()
{
    if (!stream.is_in_use())
    {
        errno = EINVAL;
        return -1;
    }

    stream.unset_flags(_IOEOF);

    // If seeking relative to the current location, then convert to a seek
    // relative to the beginning of the file.  This accounts for buffering,
    // etc., by letting fseek() tell us where we are:
    if (whence == SEEK_CUR)
    {
        offset += _ftelli64_nolock(stream.public_stream());
        whence = SEEK_SET;
    }

    __acrt_stdio_flush_nolock(stream.public_stream());

    // If the file was opened for read/write, clear flags since we don't know
    // what the user will do next with the file.  If the file was opened for
    // read only access, decrease the _bufsize so that the next call to
    // __acrt_stdio_refill_and_read_{narrow,wide}_nolock won't cost quite so
    // much:
    if (stream.has_all_of(_IOUPDATE))
    {
        stream.unset_flags(_IOWRITE | _IOREAD);
    }
    else if (stream.has_all_of(_IOREAD | _IOBUFFER_CRT) && !stream.has_any_of(_IOBUFFER_SETVBUF))
    {
        stream->_bufsiz = _SMALL_BUFSIZ;
    }

    if (_lseeki64(_fileno(stream.public_stream()), offset, whence) == -1)
        return -1;

    return 0;
}



// Repositions the file pointer of a stream to the specified location, relative
// to 'whence', which can be SEEK_SET (the beginning of the file), SEEK_CUR (the
// current pointer position), or SEEK_END (the end of the file).  The offset may
// be negative.  Returns 0 on success; returns -1 and sets errno on failure.
static int __cdecl common_fseek(__crt_stdio_stream const stream, __int64 const offset, int const whence) throw()
{
    _VALIDATE_RETURN(stream.valid(), EINVAL, -1);
    _VALIDATE_RETURN(whence == SEEK_SET || whence == SEEK_CUR || whence == SEEK_END, EINVAL, -1);

    int return_value = -1;

    _lock_file(stream.public_stream());
    __try
    {
        return_value = common_fseek_nolock(stream, offset, whence);
    }
    __finally
    {
        _unlock_file(stream.public_stream());
    }

    return return_value;
}



extern "C" int __cdecl fseek(
    FILE* const public_stream,
    long  const offset,
    int   const whence
    )
{
    return common_fseek(__crt_stdio_stream(public_stream), offset, whence);
}



extern "C" int __cdecl _fseek_nolock(
    FILE* const public_stream,
    long  const offset,
    int   const whence
    )
{
    return common_fseek_nolock(__crt_stdio_stream(public_stream), offset, whence);
}



extern "C" int __cdecl _fseeki64(
    FILE*   const public_stream,
    __int64 const offset,
    int     const whence
    )
{
    return common_fseek(__crt_stdio_stream(public_stream), offset, whence);
}



extern "C" int __cdecl _fseeki64_nolock(
    FILE*   const public_stream,
    __int64 const offset,
    int     const whence
    )
{
    return common_fseek_nolock(__crt_stdio_stream(public_stream), offset, whence);
}