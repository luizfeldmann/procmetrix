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
    PROCMETRIX_API procmetrix_error_t procmetrix_impl_linux_proc_pid_stat_read_ppid(const char *stat_data, procmetrix_pid_t *ppid);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _PROCMETRIX_PROC_LINUX_INTERNAL_H_
