#ifndef PROCMETRIX_COMMON_LINUX_INTERNAL_H
#define PROCMETRIX_COMMON_LINUX_INTERNAL_H

// Lib
#include <procmetrix/api.h>

// STD
#include <stdio.h>

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

    //! Opens a file as readonly and close-on-exec
    //! @private
    PROCMETRIX_API FILE*
    procmetrix_impl_linux_open_file_rdonly_cloexec(const char* path);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // PROCMETRIX_COMMON_LINUX_INTERNAL_H
