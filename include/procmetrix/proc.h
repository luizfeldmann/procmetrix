//! @file
//! @ingroup proc
//! @brief Functions to retrieve process information.

#ifndef PROCMETRIX_PROC_H
#define PROCMETRIX_PROC_H

// Lib
#include <procmetrix/error.h>

// STD
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus
    //! @addtogroup proc
    //! @{

    //! Type of process ID
    typedef uint32_t procmetrix_pid_t;

    //! Gets the ID of the current process
    PROCMETRIX_API procmetrix_pid_t procmetrix_get_pid();

    //! Checks if a process ID exists.
    PROCMETRIX_API bool procmetrix_pid_exists(procmetrix_pid_t pid);

    //! Allocates and fills a list of running PIDs.
    //! @details Caller must later free the list with procmetrix_free_pids.
    //! @param[out] list Pointer where the output array will be allocated.
    //!                  Must be initialized to NULL before the call.
    //! @param[out] out_count Pointer to receive the number of returned items.
    PROCMETRIX_API procmetrix_error_t
    procmetrix_list_pids(procmetrix_pid_t** list, size_t* out_count);

    //! Frees the list allocated with procmetrix_list_pids.
    //! @details Safe to call with NULL.
    PROCMETRIX_API void procmetrix_free_pids(procmetrix_pid_t** list);

    //! Gets the PID of the parent process.
    //! @param[in] pid ID of the process to inspect.
    //! @param[out] ppid Receives the parent process ID.
    PROCMETRIX_API procmetrix_error_t procmetrix_get_proc_parent_pid(
        procmetrix_pid_t pid, procmetrix_pid_t* ppid);

    //! Gets the process' name.
    //! @param[in] pid ID of the process to find the name.
    //! @param[out] name Receives the name of the process.
    //! @param[in] len Capacity of the name buffer.
    PROCMETRIX_API procmetrix_error_t
    procmetrix_get_proc_name(procmetrix_pid_t pid, char* name, size_t len);

    //! Gets the process' executable's full path.
    //! @param[in] pid ID of the process to find the executable.
    //! @param[out] path Receives the executable path.
    //! @param[in] len Capacity of the path buffer.
    PROCMETRIX_API procmetrix_error_t
    procmetrix_get_proc_exe(procmetrix_pid_t pid, char* path, size_t len);

    //! Gets the process' current working directory.
    //! @param[in] pid ID of the process to find the working dir.
    //! @param[out] cwd Receives the working dir.
    //! @param[in] len Capacity of the path buffer.
    PROCMETRIX_API procmetrix_error_t
    procmetrix_get_proc_cwd(procmetrix_pid_t pid, char* cwd, size_t len);

    //! List of command line argluments for a process
    typedef struct procmetrix_proc_cmdline
    {
        //! Number of arguments
        size_t argc;

        //! Values of the arguments
        char** argv;
    } procmetrix_proc_cmdline_t;

    //! Gets the process command line.
    //! @details The list must be free'd by the caller.
    //! @param[in] pid ID of the process to find the command line.
    //! @param[out] cmdline Receives number and value of
    //!                     the command line arguments.
    PROCMETRIX_API procmetrix_error_t procmetrix_get_proc_cmdline(
        procmetrix_pid_t pid, procmetrix_proc_cmdline_t* cmdline);

    //! Frees the command line arguments list struct.
    PROCMETRIX_API void
    procmetrix_free_proc_cmdline(procmetrix_proc_cmdline_t* cmd_line);

    //! Key-value pair for one environment variable in a process
    typedef struct procmetrix_proc_environ_var
    {
        //! Key/name of the environment variable
        char* name;

        //! Value of the environment variable
        char* value;
    } procmetrix_proc_environ_var_t;

    //! List of environment variables for a process
    typedef struct procmetrix_proc_environ
    {
        //! Number of items.
        size_t count;

        //! Array of the environment variables
        procmetrix_proc_environ_var_t* vars;
    } procmetrix_proc_environ_t;

    //! Gets the process environment.
    //! @details The list must be free'd by the caller.
    //! @param[in] pid ID of the process to find the environment.
    //! @param[out] proc_environ Receives number and key-value pairs
    //!                          for the environment variables.
    PROCMETRIX_API procmetrix_error_t procmetrix_get_proc_environ(
        procmetrix_pid_t pid, procmetrix_proc_environ_t* proc_environ);

    //! Frees the environment list struct.
    PROCMETRIX_API void
    procmetrix_free_proc_environ(procmetrix_proc_environ_t* proc_environ);

    //! Memory information about a process
    typedef struct procmetrix_proc_memory_info
    {
        //! Resident Set Size
        //! Non-swapped physical memory a process has used
        //! On UNIX it matches RES column in "top"
        //! On Windows this is an alias for wset field
        //! and it matches "Mem Usage" of task manager
        uint64_t rss;

        //! Virtual Memory Size
        //! Total amount of virtual memory used by the process
        //! On UNIX it matches VIRT column in "top"
        //! On Windows this is an alias for pagefile field
        uint64_t vms;

        //! Memory that could be potentially shared with other processes
        //! Matches SHR column in "top"
        //! (Linux only)
        uint64_t shared;

        //! The amount of memory devoted to executable code
        //! Matches CODE column in "top"
        //! (Linux only)
        uint64_t text;

        //! The memory used by shared libraries
        //! (Linux only)
        uint64_t lib;

        //! DRS (data resident set)
        //! The amount of physical memory devoted to other than executable code
        //! Matches DATA column in "top"
        //! (Linux only)
        uint64_t data;

        //! The number of dirty pages
        //! (Linux only)
        uint64_t dirty;
    } procmetrix_proc_memory_info_t;

    //! Reads the memory information of a process.
    //! @param[in] pid ID of the process to get the memory info.
    //! @param[out] memory_info Receives the retrieved memory info.
    PROCMETRIX_API procmetrix_error_t procmetrix_get_proc_memory_info(
        procmetrix_pid_t pid, procmetrix_proc_memory_info_t* memory_info);

    //! Accumulated process times, in seconds
    typedef struct procmetrix_proc_cpu_times
    {
        //! Time spent in user mode.
        double user;

        //! time spent in kernel mode.
        double system;

        //! User time of all child processes
        //! (Linux only)
        double children_user;

        //! system time of all child processes
        //! (Linux only)
        double children_system;
    } procmetrix_proc_cpu_times_t;

    //! Reads the process' usage of CPU time.
    //! @param[in] pid ID of the process to get the CPU times.
    //! @param[out] cpu_times Receives the retrieved CPU times.
    PROCMETRIX_API procmetrix_error_t procmetrix_get_proc_cpu_times(
        procmetrix_pid_t pid, procmetrix_proc_cpu_times_t* cpu_times);

    //! Calculates the difference in CPU times between two measurements.
    //! @details Negative values are clamped.
    //! @param[in] before Previous CPU times snapshot.
    //! @param[in] after Later CPU times snapshot.
    //! @param[out] delta Receives the calculate times delta.
    PROCMETRIX_API procmetrix_error_t procmetrix_proc_cpu_times_delta(
        const procmetrix_proc_cpu_times_t* before,
        const procmetrix_proc_cpu_times_t* after,
        procmetrix_proc_cpu_times_t* delta);

    //! Calculates the total sum of CPU time spent by a process.
    PROCMETRIX_API double
    procmetrix_proc_cpu_times_sum(const procmetrix_proc_cpu_times_t* cpu_times);

    // clang-format off
    //! @}
    // clang-format on
#ifdef __cplusplus
}
#endif // __cplusplus

#endif // PROCMETRIX_PROC_H
