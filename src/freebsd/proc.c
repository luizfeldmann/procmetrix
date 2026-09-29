// Lib
#include "procmetrix/error.h"
#include <procmetrix/proc.h>

// Internal
#include <internal/freebsd/common_freebsd_internal.h>

// STD
#include <stdlib.h>
#include <string.h>

// System
#include <libutil.h>
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
    int cntp = 0;
    struct kinfo_proc* proc_list = kinfo_getallproc(&cntp);
    if (NULL == proc_list)
        return PROCMETRIX_ERROR_UNKNOWN;

    // Alloc output array
    procmetrix_pid_t* pids = calloc(cntp, sizeof(procmetrix_pid_t));
    if (NULL == pids)
    {
        free(proc_list);
        return PROCMETRIX_ERROR_OUT_OF_MEMORY;
    }

    for (size_t i = 0; i < cntp; ++i)
        pids[i] = proc_list[i].ki_pid;

    *list = pids;
    *out_count = cntp;

    // Cleanup sysctl data
    free(proc_list);

    return PROCMETRIX_ERROR_NONE;
}

procmetrix_error_t
procmetrix_get_proc_parent_pid(procmetrix_pid_t pid, procmetrix_pid_t* ppid)
{
    // Sanity
    if (NULL == ppid)
        return PROCMETRIX_ERROR_INVALID_ARGUMENT;
    *ppid = 0;

    if (0 == pid)
        return PROCMETRIX_ERROR_INVALID_ARGUMENT;

    // Read process info
    struct kinfo_proc* info = kinfo_getproc((pid_t)pid);
    if (NULL == info)
        return PROCMETRIX_ERROR_UNKNOWN;

    *ppid = info->ki_ppid;

    // Cleanup
    free(info);

    return PROCMETRIX_ERROR_NONE;
}

procmetrix_error_t
procmetrix_get_proc_name(procmetrix_pid_t pid, char* name, size_t len)
{
    // Sanity
    if (NULL == name || 0 == len)
        return PROCMETRIX_ERROR_INVALID_ARGUMENT;

    if (0 == pid)
    {
        memset(name, 0, len);
        return PROCMETRIX_ERROR_INVALID_ARGUMENT;
    }

    // Read process info
    struct kinfo_proc* info = kinfo_getproc((pid_t)pid);
    if (NULL == info)
    {
        memset(name, 0, len);
        return PROCMETRIX_ERROR_UNKNOWN;
    }

    // Copy process name to output
    strncpy(name, info->ki_comm, len);
    name[len - 1] = '\0';

    // Cleanup
    free(info);

    return PROCMETRIX_ERROR_NONE;
}

procmetrix_error_t
procmetrix_get_proc_exe(procmetrix_pid_t pid, char* path, size_t len)
{
    // Sanity
    if (NULL == path || 0 == len)
        return PROCMETRIX_ERROR_INVALID_ARGUMENT;
    memset(path, 0, len);

    if (0 == pid)
        return PROCMETRIX_ERROR_INVALID_ARGUMENT;

    // Read from sysctl
    int mib[4] = { CTL_KERN, KERN_PROC, KERN_PROC_PATHNAME, (int)pid };
    return procmetrix_impl_bsd_sysctl(mib, 4, path, len);
}

procmetrix_error_t
procmetrix_get_proc_cwd(procmetrix_pid_t pid, char* dst_cwd, size_t dst_len)
{
    // Sanity
    if (NULL == dst_cwd || 0 == dst_len)
        return PROCMETRIX_ERROR_INVALID_ARGUMENT;
    memset(dst_cwd, 0, dst_len);

    if (0 == pid)
        return PROCMETRIX_ERROR_INVALID_ARGUMENT;

    // Read open files
    int cntp = 0;
    struct kinfo_file* files = kinfo_getfile((pid_t)pid, &cntp);
    if (NULL == files)
        return PROCMETRIX_ERROR_UNKNOWN;

    // Iterate to find the CWD file
    procmetrix_error_t status = PROCMETRIX_ERROR_FILE_READ;
    for (int i = 0; i < cntp; ++i)
    {
        if (files[i].kf_fd == KF_FD_TYPE_CWD)
        {
            strncpy(dst_cwd, files[i].kf_path, dst_len);
            dst_cwd[dst_len - 1] = '\0';
            status = PROCMETRIX_ERROR_NONE;
            break;
        }
    }

    // Cleanup
    free(files);

    return status;
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
