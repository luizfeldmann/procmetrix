// Internal
#include <internal/algo.h>
#include <internal/linux/common_linux_internal.h>
#include <internal/linux/proc_linux_internal.h>

// STD
#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Linux
#include <glob.h>
#include <linux/limits.h>
#include <signal.h>
#include <unistd.h>

// Util

static FILE *procmetrix_impl_linux_proc_pid_open_file(const char *filename, procmetrix_pid_t pid)
{
    // Sanity
    if (NULL == filename)
        return NULL;

    // Path to the file
    char read_path[PATH_MAX];
    snprintf(read_path, sizeof(read_path), "/proc/%" PRIu32 "/%s", pid, filename);

    // Open the file
    return procmetrix_impl_linux_open_file_rdonly_cloexec(read_path);
}

static procmetrix_error_t procmetrix_impl_linux_proc_pid_read_line(FILE *read_file, char *buf, size_t len)
{
    // Sanity
    if (NULL == read_file || NULL == buf || 0 == len)
        return PROCMETRIX_ERROR_INVALID_ARGUMENT;

    // Read the file line
    procmetrix_error_t status = PROCMETRIX_ERROR_NONE;
    if (fgets(buf, (int)len, read_file) == NULL)
        status = PROCMETRIX_ERROR_MALFORMED;
    else
    {
        // Remove trailing newline
        buf[strcspn(buf, "\n")] = '\0';
    }

    return status;
}

static procmetrix_error_t procmetrix_impl_linux_read_full_file(FILE *read_file, char **buffer, size_t *file_size)
{
    // Sanity
    if (NULL == read_file || NULL == buffer || NULL == file_size)
        return PROCMETRIX_ERROR_INVALID_ARGUMENT;

    // Initial buffer allocation
    size_t chunk = 0;
    size_t capacity = 128;

    *file_size = 0;
    *buffer = (char *)malloc(capacity);

    if (NULL == *buffer)
        return PROCMETRIX_ERROR_OUT_OF_MEMORY;

    // Read file chunks
    procmetrix_error_t status = PROCMETRIX_ERROR_NONE;
    while ((chunk = fread(*buffer + *file_size, 1, capacity - *file_size, read_file)) > 0)
    {
        *file_size += chunk;

        // Grow buffer capacity geometrically
        if (*file_size >= capacity)
        {
            capacity *= 2;

            // Check rellocation success
            char *new_buffer = (char *)realloc(*buffer, capacity);
            if (NULL == new_buffer)
            {
                status = PROCMETRIX_ERROR_OUT_OF_MEMORY;
                break;
            }

            // Continue
            *buffer = new_buffer;
        }
    }

    // Check if the loop exited due to error
    if (ferror(read_file))
        status = PROCMETRIX_ERROR_FILE_READ;

    // Cleanup in case of error
    if (PROCMETRIX_ERROR_NONE != status)
    {
        free(*buffer);
        *buffer = NULL;
        *file_size = 0;
    }

    return status;
}

static procmetrix_error_t procmetrix_impl_linux_read_zero_terminated_tokens(FILE *read_file, char ***tokens, size_t *num_tokens)
{
    // Read the file
    size_t file_size = 0;
    char *buffer = NULL;
    procmetrix_error_t status = procmetrix_impl_linux_read_full_file(read_file, &buffer, &file_size);
    if (status != PROCMETRIX_ERROR_NONE)
        return status;

    // Split the tokens
    status = procmetrix_split_zero_terminated_tokens(buffer, file_size, tokens, num_tokens);

    // Cleanup the buffer
    free(buffer);

    return status;
}

static procmetrix_error_t procmetrix_impl_linux_proc_pid_follow_symlink(const char *filename, procmetrix_pid_t pid, char *buf, size_t len)
{
    // Sanity
    if (NULL == buf || 0 == len)
        return PROCMETRIX_ERROR_INVALID_ARGUMENT;

    // Defensively clear result
    memset(buf, 0, len);

    // Sanity (2)
    if (NULL == filename || 0 == pid)
        return PROCMETRIX_ERROR_INVALID_ARGUMENT;

    // Path to the symlink
    char link_path[PATH_MAX];
    snprintf(link_path, sizeof(link_path),
             "/proc/%" PRIu32 "/%s", pid, filename);

    // Read the symlink
    size_t read_count = readlink(link_path, buf, len - 1);
    if (read_count < 0)
        return PROCMETRIX_ERROR_FILE_READ;

    // Safe truncation
    buf[read_count] = '\0';

    return PROCMETRIX_ERROR_NONE;
}

// Private Impl

procmetrix_error_t procmetrix_impl_linux_proc_pid_stat_read_ppid(const char *stat_data, procmetrix_pid_t *ppid)
{
    // Sanity
    if (NULL == ppid)
        return PROCMETRIX_ERROR_INVALID_ARGUMENT;

    *ppid = 0;

    if (NULL == stat_data)
        return PROCMETRIX_ERROR_INVALID_ARGUMENT;

    // Find delimiter
    const char *rparen = strrchr(stat_data, ')');
    if (NULL == rparen)
        return PROCMETRIX_ERROR_MALFORMED;

    if (sscanf(rparen + 1, " %*c %" SCNu32, ppid) != 1)
        return PROCMETRIX_ERROR_MALFORMED;

    return PROCMETRIX_ERROR_NONE;
}

procmetrix_error_t procmetrix_impl_linux_proc_pid_stat_read_cpu_times(const char *stat_data, procmetrix_proc_cpu_times_t *cpu_times)
{
    // Sanity
    if (NULL == cpu_times)
        return PROCMETRIX_ERROR_INVALID_ARGUMENT;

    memset(cpu_times, 0, sizeof(*cpu_times));

    if (NULL == stat_data)
        return PROCMETRIX_ERROR_INVALID_ARGUMENT;

    // Jiffies conversion
    long ticks_per_second = sysconf(_SC_CLK_TCK);
    if (ticks_per_second <= 0)
        return PROCMETRIX_ERROR_UNKNOWN;

    // Find delimiter
    const char *rparen = strrchr(stat_data, ')');
    if (NULL == rparen)
        return PROCMETRIX_ERROR_MALFORMED;

    uint64_t user = 0;
    uint64_t system = 0;
    int64_t children_user = 0;
    int64_t children_system = 0;
    if (4 != sscanf(rparen + 1,
                    " %*c"       // state
                    " %*d"       // ppid
                    " %*d"       // pgrp
                    " %*d"       // session
                    " %*d"       // tty_nr
                    " %*d"       // tpgid
                    " %*u"       // flags
                    " %*u"       // minflt
                    " %*u"       // cminflt
                    " %*u"       // majflt
                    " %*u"       // cmajflt
                    " %" SCNu64  // utime
                    " %" SCNu64  // stime
                    " %" SCNd64  // cutime
                    " %" SCNd64, // cstime
                    &user, &system, &children_user, &children_system))
        return PROCMETRIX_ERROR_MALFORMED;

    // Convert ticks to seconds
    cpu_times->user = (double)user / (double)ticks_per_second;
    cpu_times->system = (double)system / (double)ticks_per_second;
    cpu_times->children_user = (double)children_user / (double)ticks_per_second;
    cpu_times->children_system = (double)children_system / (double)ticks_per_second;

    return PROCMETRIX_ERROR_NONE;
}

procmetrix_error_t procmetrix_impl_linux_proc_read_statm(const char *statm_data, procmetrix_proc_memory_info_t *memory_info)
{
    // Sanity
    if (NULL == memory_info)
        return PROCMETRIX_ERROR_INVALID_ARGUMENT;

    memset(memory_info, 0, sizeof(*memory_info));

    if (NULL == statm_data)
        return PROCMETRIX_ERROR_INVALID_ARGUMENT;

    // Read fields
    uint64_t vms = 0, rss = 0, shared = 0, text = 0, lib = 0, data = 0, dirty = 0; // NOLINT(readability-isolate-declaration)
    if (7 != sscanf(statm_data, "%" SCNu64 " %" SCNu64 " %" SCNu64 " %" SCNu64 " %" SCNu64 " %" SCNu64 " %" SCNu64,
                    &vms, &rss, &shared, &text, &lib, &data, &dirty))
        return PROCMETRIX_ERROR_MALFORMED;

    // Convert pages to bytes
    uint64_t page_size = (uint64_t)sysconf(_SC_PAGE_SIZE);

    memory_info->vms = page_size * vms;
    memory_info->rss = page_size * rss;
    memory_info->shared = page_size * shared;
    memory_info->text = page_size * text;
    memory_info->lib = page_size * lib;
    memory_info->data = page_size * data;
    memory_info->dirty = page_size * dirty;

    return PROCMETRIX_ERROR_NONE;
}

// Public Impl

procmetrix_pid_t procmetrix_get_pid()
{
    return getpid();
}

bool procmetrix_pid_exists(procmetrix_pid_t pid)
{
    // Process zero is the kernel, not user space
    if (pid == 0)
        return false;

    // Kill with signal Zero is an existential test
    if (kill((pid_t)pid, 0) == 0)
        return true;

    // Process exists but we don't have permission for it
    if (EPERM == errno)
        return true;

    // Doesn't exist
    return false;
}

procmetrix_error_t procmetrix_list_pids(procmetrix_pid_t **list, size_t *out_count)
{
    // Sanity
    if (list == NULL || *list != NULL || out_count == NULL)
        return PROCMETRIX_ERROR_INVALID_ARGUMENT;

    // Don't let this keep garbage
    *out_count = 0;

    // Glob the files
    glob_t glb;
    if (0 != glob("/proc/[0-9]*", 0, NULL, &glb))
        return PROCMETRIX_ERROR_FILE_READ;

    // Allocate output list
    procmetrix_pid_t *pids = (procmetrix_pid_t *)calloc(glb.gl_pathc, sizeof(procmetrix_pid_t));
    *list = pids;
    if (NULL == pids)
    {
        globfree(&glb);
        return PROCMETRIX_ERROR_OUT_OF_MEMORY;
    }

    // Fill the list
    procmetrix_error_t status = PROCMETRIX_ERROR_NONE;
    for (*out_count = 0; *out_count < glb.gl_pathc; ++(*out_count))
    {
        if (1 != sscanf(glb.gl_pathv[*out_count], "/proc/%" SCNu32, &pids[*out_count]))
            status = PROCMETRIX_ERROR_MALFORMED;
    }

    // Cleanup
    globfree(&glb);

    return status;
}

procmetrix_error_t procmetrix_get_proc_parent_pid(procmetrix_pid_t pid, procmetrix_pid_t *ppid)
{
    // Sanity
    if (NULL == ppid)
        return PROCMETRIX_ERROR_INVALID_ARGUMENT;

    // Consistent result even in case of error
    *ppid = 0;

    if (0 == pid)
        return PROCMETRIX_ERROR_INVALID_ARGUMENT;

    // Open file
    FILE *read_file = procmetrix_impl_linux_proc_pid_open_file("stat", pid);
    if (NULL == read_file)
        return PROCMETRIX_ERROR_FILE_READ;

    // Read the input file
    char buf[4096];
    procmetrix_error_t status = procmetrix_impl_linux_proc_pid_read_line(read_file, buf, sizeof(buf));
    if (PROCMETRIX_ERROR_NONE == status)
    {
        // Invoke impl
        status = procmetrix_impl_linux_proc_pid_stat_read_ppid(buf, ppid);
    }

    // Cleanup
    fclose(read_file);

    return status;
}

procmetrix_error_t procmetrix_get_proc_name(procmetrix_pid_t pid, char *name, size_t len)
{
    // Sanity
    if (NULL == name || 0 == len)
        return PROCMETRIX_ERROR_INVALID_ARGUMENT;

    // Defensively clear result
    memset(name, 0, len);

    if (0 == pid)
        return PROCMETRIX_ERROR_INVALID_ARGUMENT;

    // Open file
    FILE *read_file = procmetrix_impl_linux_proc_pid_open_file("comm", pid);
    if (NULL == read_file)
        return PROCMETRIX_ERROR_FILE_READ;

    // Read line content
    procmetrix_error_t result = procmetrix_impl_linux_proc_pid_read_line(read_file, name, len);

    // Cleanup
    fclose(read_file);

    return result;
}

procmetrix_error_t procmetrix_get_proc_exe(procmetrix_pid_t pid, char *path, size_t len)
{
    return procmetrix_impl_linux_proc_pid_follow_symlink("exe", pid, path, len);
}

procmetrix_error_t procmetrix_get_proc_cwd(procmetrix_pid_t pid, char *cwd, size_t len)
{
    return procmetrix_impl_linux_proc_pid_follow_symlink("cwd", pid, cwd, len);
}

procmetrix_error_t procmetrix_get_proc_cmdline(procmetrix_pid_t pid, procmetrix_proc_cmdline_t *cmdline)
{
    // Sanity (1)
    if (NULL == cmdline)
        return PROCMETRIX_ERROR_INVALID_ARGUMENT;

    // Consistent result even if error
    memset(cmdline, 0, sizeof(*cmdline));

    // Sanity (2)
    if (0 == pid)
        return PROCMETRIX_ERROR_INVALID_ARGUMENT;

    // Open the file
    FILE *read_file = procmetrix_impl_linux_proc_pid_open_file("cmdline", pid);
    if (NULL == read_file)
        return PROCMETRIX_ERROR_FILE_READ;

    // The command line arguments are the split tokens
    procmetrix_error_t status = procmetrix_impl_linux_read_zero_terminated_tokens(
        read_file, &(cmdline->argv), &(cmdline->argc));

    // Cleanup
    fclose(read_file);

    return status;
}

procmetrix_error_t procmetrix_get_proc_environ(procmetrix_pid_t pid, procmetrix_proc_environ_t *environ)
{
    // Sanity (1)
    if (NULL == environ)
        return PROCMETRIX_ERROR_INVALID_ARGUMENT;

    // Consistent result even if error
    memset(environ, 0, sizeof(*environ));

    // Sanity (2)
    if (0 == pid)
        return PROCMETRIX_ERROR_INVALID_ARGUMENT;

    // Open the file
    FILE *read_file = procmetrix_impl_linux_proc_pid_open_file("environ", pid);
    if (NULL == read_file)
        return PROCMETRIX_ERROR_FILE_READ;

    // Tokenize the environment variables key=value pairs
    size_t numvars = 0;
    char **varlines = NULL;

    procmetrix_error_t status = procmetrix_impl_linux_read_zero_terminated_tokens(
        read_file, &varlines, &numvars);

    // Cleanup file
    fclose(read_file);

    // Allocate space for key-value pairs
    if (PROCMETRIX_ERROR_NONE == status)
        status = procmetrix_split_environ_vars((const char *const *)varlines, numvars, environ);

    // Cleanup temp env vars line buffers
    for (size_t i = 0; i < numvars; ++i)
        free(varlines[i]);
    free(varlines);

    return status;
}

procmetrix_error_t procmetrix_get_proc_memory_info(procmetrix_pid_t pid, procmetrix_proc_memory_info_t *memory_info)
{
    // Sanity
    if (NULL == memory_info)
        return PROCMETRIX_ERROR_INVALID_ARGUMENT;

    // Defensively clear result
    memset(memory_info, 0, sizeof(*memory_info));

    if (0 == pid)
        return PROCMETRIX_ERROR_INVALID_ARGUMENT;

    // Open file
    FILE *read_file = procmetrix_impl_linux_proc_pid_open_file("statm", pid);
    if (NULL == read_file)
        return PROCMETRIX_ERROR_FILE_READ;

    // Read line content
    char buf[128];
    procmetrix_error_t result = procmetrix_impl_linux_proc_pid_read_line(read_file, buf, sizeof(buf));

    // Cleanup
    fclose(read_file);

    // Invoke impl
    if (PROCMETRIX_ERROR_NONE == result)
        result = procmetrix_impl_linux_proc_read_statm(buf, memory_info);

    return result;
}

procmetrix_error_t procmetrix_get_proc_cpu_times(procmetrix_pid_t pid, procmetrix_proc_cpu_times_t *cpu_times)
{
    // Sanity
    if (NULL == cpu_times)
        return PROCMETRIX_ERROR_INVALID_ARGUMENT;

    // Defensively clear result
    memset(cpu_times, 0, sizeof(*cpu_times));

    if (0 == pid)
        return PROCMETRIX_ERROR_INVALID_ARGUMENT;

    // Open file
    FILE *read_file = procmetrix_impl_linux_proc_pid_open_file("stat", pid);
    if (NULL == read_file)
        return PROCMETRIX_ERROR_FILE_READ;

    // Read the input file
    char buf[4096];
    procmetrix_error_t status = procmetrix_impl_linux_proc_pid_read_line(read_file, buf, sizeof(buf));
    if (PROCMETRIX_ERROR_NONE == status)
    {
        // Invoke impl
        status = procmetrix_impl_linux_proc_pid_stat_read_cpu_times(buf, cpu_times);
    }

    // Cleanup
    fclose(read_file);

    return status;
}
