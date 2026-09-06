#ifndef _PROCMETRIX_ERROR_H_
#define _PROCMETRIX_ERROR_H_

#include <procmetrix/api.h>

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

    //! Error codes for the library
    typedef enum procmetrix_error
    {
        //! Success.
        PROCMETRIX_ERROR_NONE = 0,

        //! Unknown error
        PROCMETRIX_ERROR_UNKNOWN,

        //! Invalid argument provided, such as a null pointer.
        PROCMETRIX_ERROR_INVALID_ARGUMENT,

        //! The full result is actually larger then what was returned.
        //! The provided buffer was too small.
        PROCMETRIX_ERROR_MORE_DATA,

        //! Unable to open the file for reading
        PROCMETRIX_ERROR_FILE_READ,

        //! Unable to parse the contents of some data returned by the operating system.
        PROCMETRIX_ERROR_MALFORMED,

        //! Failed allocation.
        PROCMETRIX_ERROR_OUT_OF_MEMORY,

        //! The function isn't implemented for this platform.
        PROCMETRIX_NOT_IMPLEMENTED,
    } procmetrix_error_t;

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _PROCMETRIX_ERROR_H_
