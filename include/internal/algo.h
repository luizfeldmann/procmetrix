#ifndef _PROCMETRIX_ALGORITHM_INTERNAL_H_
#define _PROCMETRIX_ALGORITHM_INTERNAL_H_

// Lib
#include <procmetrix/proc.h>

// STD
#include <stdio.h>

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

    //! Splits the input buffer by '\0' terminated tokens
    //! @private
    PROCMETRIX_API procmetrix_error_t procmetrix_split_zero_terminated_tokens(const char *buffer, size_t buffer_size, char ***tokens, size_t *num_tokens);

    //! Splits the array of '=' delimited key-value pairs as environment variables
    //! @private
    PROCMETRIX_API procmetrix_error_t procmetrix_split_environ_vars(const char *const *varlines, size_t numvars, procmetrix_proc_environ_t *environment);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _PROCMETRIX_ALGORITHM_INTERNAL_H_
