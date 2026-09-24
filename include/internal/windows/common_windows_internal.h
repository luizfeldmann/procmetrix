#ifndef PROCMETRIX_COMMON_WINDOWS_INTERNAL_H
#define PROCMETRIX_COMMON_WINDOWS_INTERNAL_H

// Lib
#include <procmetrix/api.h>

// Windows
#include <Windows.h>

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

    //! Opens a file as readonly and close-on-exec
    //! @private
    PROCMETRIX_API double procmetrix_impl_windows_large_uint_to_secs(const ULARGE_INTEGER *li);

    PROCMETRIX_API double procmetrix_impl_windows_large_int_to_secs(const LARGE_INTEGER *li);

    PROCMETRIX_API double procmetrix_impl_windows_filetime_to_secs(const FILETIME *pft);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // PROCMETRIX_COMMON_WINDOWS_INTERNAL_H
