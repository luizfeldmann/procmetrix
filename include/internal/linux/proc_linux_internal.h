#ifndef _PROCMETRIX_PROC_LINUX_INTERNAL_H_
#define _PROCMETRIX_PROC_LINUX_INTERNAL_H_

// Lib
#include <procmetrix/proc.h>

// STD
#include <stdio.h>

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

    //! Reads the parent process ID from /proc/<pid>/stat
    //! @private
    PROCMETRIX_API procmetrix_error_t procmetrix_impl_linux_proc_pid_stat_read_ppid(const char *stat_data, procmetrix_pid_t *ppid);

    //! Reads CPU times from /proc/<pid>/stat
    //! @private
    PROCMETRIX_API procmetrix_error_t procmetrix_impl_linux_proc_pid_stat_read_cpu_times(const char *stat_data, procmetrix_proc_cpu_times_t *cpu_times);

    //! Reads the memory information from /proc/<pid>/statm
    //! @private
    PROCMETRIX_API procmetrix_error_t procmetrix_impl_linux_proc_read_statm(const char *statm_data, procmetrix_proc_memory_info_t *memory_info);

    //! Splits the input buffer by '\0' terminated tokens
    //! @private
    PROCMETRIX_API procmetrix_error_t procmetrix_impl_linux_split_zero_terminated_tokens(const char *buffer, size_t buffer_size, char ***tokens, size_t *num_tokens);

    //! Splits the array of '=' delimited key-value pairs as environment variables
    //! @private
    PROCMETRIX_API procmetrix_error_t procmetrix_impl_linux_split_environ_vars(const char *const *varlines, size_t numvars, procmetrix_proc_environ_t *environ);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _PROCMETRIX_PROC_LINUX_INTERNAL_H_
