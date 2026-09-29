// Lib
#include "procmetrix/error.h"
#include <procmetrix/proc.h>

// Internal
#include <internal/freebsd/common_freebsd_internal.h>

// STD
#include <stdlib.h>

// System
#include <sys/sysctl.h>
#include <sys/types.h>
#include <sys/user.h>

// Impl

procmetrix_error_t
procmetrix_list_pids(procmetrix_pid_t** list, size_t* out_count)
{
    // Sanity
    if (NULL == list || NULL == out_count)
        return PROCMETRIX_ERROR_INVALID_ARGUMENT;

    // Defensive
    *out_count = 0;
    *list = NULL;

    // Read list of process infos
    size_t buflen = 0;
    struct kinfo_proc* proc_list = NULL;

    int mib[4] = { CTL_KERN, KERN_PROC, KERN_PROC_PROC, 0 };
    procmetrix_error_t status =
        procmetrix_impl_bsd_sysctl_alloc(mib, 4, (void**)&proc_list, &buflen);

    if (PROCMETRIX_ERROR_NONE != status)
        return status;

    // Alloc output array
    size_t num_procs = buflen / sizeof(*proc_list);
    procmetrix_pid_t* pids = calloc(num_procs, sizeof(procmetrix_pid_t));

    if (NULL == pids)
        status = PROCMETRIX_ERROR_OUT_OF_MEMORY;
    else
    {
        *list = pids;
        *out_count = num_procs;

        for (size_t i = 0; i < num_procs; ++i)
            pids[i] = proc_list[i].ki_pid;
    }

    // Cleanup sysctl data
    free(proc_list);

    return status;
}

procmetrix_error_t
procmetrix_get_proc_parent_pid(procmetrix_pid_t pid, procmetrix_pid_t* ppid)
{
    return PROCMETRIX_NOT_IMPLEMENTED;
}

procmetrix_error_t
procmetrix_get_proc_name(procmetrix_pid_t pid, char* name, size_t len)
{
    return PROCMETRIX_NOT_IMPLEMENTED;
}

procmetrix_error_t
procmetrix_get_proc_exe(procmetrix_pid_t pid, char* path, size_t len)
{
    return PROCMETRIX_NOT_IMPLEMENTED;
}

procmetrix_error_t
procmetrix_get_proc_cwd(procmetrix_pid_t pid, char* dst_cwd, size_t dst_len)
{
    return PROCMETRIX_NOT_IMPLEMENTED;
}

procmetrix_error_t procmetrix_get_proc_cmdline(
    procmetrix_pid_t pid, procmetrix_proc_cmdline_t* cmdline)
{
    return PROCMETRIX_NOT_IMPLEMENTED;
}

procmetrix_error_t procmetrix_get_proc_environ(
    procmetrix_pid_t pid, procmetrix_proc_environ_t* proc_environ)
{
    return PROCMETRIX_NOT_IMPLEMENTED;
}

procmetrix_error_t procmetrix_get_proc_memory_info(
    procmetrix_pid_t pid, procmetrix_proc_memory_info_t* memory_info)
{
    return PROCMETRIX_NOT_IMPLEMENTED;
}

procmetrix_error_t procmetrix_get_proc_cpu_times(
    procmetrix_pid_t pid, procmetrix_proc_cpu_times_t* cpu_times)
{
    return PROCMETRIX_NOT_IMPLEMENTED;
}
