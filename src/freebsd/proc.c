// Lib
#include <procmetrix/proc.h>

procmetrix_error_t
procmetrix_list_pids(procmetrix_pid_t** list, size_t* out_count)
{
    return PROCMETRIX_NOT_IMPLEMENTED;
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
