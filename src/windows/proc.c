// Lib
#include <procmetrix/proc.h>

procmetrix_pid_t procmetrix_get_pid()
{
    // @TODO
    return 0;
}

bool procmetrix_pid_exists(procmetrix_pid_t pid)
{
    // @TODO
    return false;
}

procmetrix_error_t procmetrix_list_pids(procmetrix_pid_t **list, size_t *out_count)
{
    // @TODO
    return PROCMETRIX_NOT_IMPLEMENTED;
}

procmetrix_error_t procmetrix_get_proc_parent_pid(procmetrix_pid_t pid, procmetrix_pid_t *ppid)
{
    // @TODO
    return PROCMETRIX_NOT_IMPLEMENTED;
}

procmetrix_error_t procmetrix_get_proc_name(procmetrix_pid_t pid, char *name, size_t len)
{
    // @TODO
    return PROCMETRIX_NOT_IMPLEMENTED;
}

procmetrix_error_t procmetrix_get_proc_exe(procmetrix_pid_t pid, char *path, size_t len)
{
    // @TODO
    return PROCMETRIX_NOT_IMPLEMENTED;
}

procmetrix_error_t procmetrix_get_proc_cwd(procmetrix_pid_t pid, char *cwd, size_t len)
{
    // @TODO
    return PROCMETRIX_NOT_IMPLEMENTED;
}

procmetrix_error_t procmetrix_get_proc_cmdline(procmetrix_pid_t pid, procmetrix_proc_cmdline_t *cmdline)
{
    // @TODO
    return PROCMETRIX_NOT_IMPLEMENTED;
}

procmetrix_error_t procmetrix_get_proc_environ(procmetrix_pid_t pid, procmetrix_proc_environ_t *environ)
{
    // @TODO
    return PROCMETRIX_NOT_IMPLEMENTED;
}

procmetrix_error_t procmetrix_get_proc_memory_info(procmetrix_pid_t pid, procmetrix_proc_memory_info_t *memory_info)
{
    // @TODO
    return PROCMETRIX_NOT_IMPLEMENTED;
}

procmetrix_error_t procmetrix_get_proc_cpu_times(procmetrix_pid_t pid, procmetrix_proc_cpu_times_t *cpu_times)
{
    // @TODO
    return PROCMETRIX_NOT_IMPLEMENTED;
}
