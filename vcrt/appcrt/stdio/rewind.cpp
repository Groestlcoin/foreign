//
// rewind.cpp
//
//      Copyright (c) Microsoft Corporation.  All rights reserved.
//
// Defines rewind(), which rewinds a stream.
//
#include <corecrt_internal_stdio.h>



// Rewinds a stream back to the beginning, if the stream supports seeking.  The
// stream is flushed and errors are cleared before the rewind.
extern "C" void __cdecl rewind(FILE* const public_stream)
{
    __crt_stdio_stream const stream(public_stream);

    _VALIDATE_RETURN_VOID(stream.valid(), EINVAL);

    int const fh = _fileno(stream.public_stream());

    _lock_file(stream.public_stream());
    __try
    {
        // Flush the streeam, reset the error state, and seek back to the
        // beginning:
        __acrt_stdio_flush_nolock(stream.public_stream());

        stream.unset_flags(_IOERROR | _IOEOF);
        _osfile_safe(fh) &= ~(FEOFLAG);

       if (stream.has_all_of(_IOUPDATE))
           stream.unset_flags(_IOREAD | _IOWRITE);
        
        if (_lseek(fh, 0, 0) == -1)
            stream.set_flags(_IOERROR);
    }
    __finally
    {
        _unlock_file(stream.public_stream());
    }
}
