// Lib
#include <procmetrix/proc.h>

// Internal
#include <internal/linux/common_linux_internal.h>
#include <internal/linux/proc_linux_internal.h>

// STD
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <inttypes.h>

// Linux
#include <glob.h>
#include <signal.h>
#include <unistd.h>
#include <linux/limits.h>

// Util

static FILE *procmetrix_impl_linux_proc_pid_open_file(const char *filename, procmetrix_pid_t pid)
{
    // Sanity
    if (NULL == filename || 0 == pid)
        return NULL;

    // Path to the file
    char read_path[PATH_MAX];
    snprintf(read_path, sizeof(read_path), "/proc/%" PRIu32 "/%s", pid, filename);

    // Open the file
    return procmetrix_impl_linux_open_file_rdonly_cloexec(read_path);
}

static procmetrix_error_t procmetrix_impl_linux_proc_pid_read_line(const char *filename, procmetrix_pid_t pid, char *buf, size_t len)
{
    // Sanity
    if (NULL == buf || 0 == len)
        return PROCMETRIX_ERROR_INVALID_ARGUMENT;

    // Defensively clear result
    memset(buf, 0, len);

    // Sanity
    if (NULL == filename || 0 == pid)
        return PROCMETRIX_ERROR_INVALID_ARGUMENT;

    // Open the file
    FILE *read_file = procmetrix_impl_linux_proc_pid_open_file(filename, pid);
    if (NULL == read_file)
        return PROCMETRIX_ERROR_FILE_READ;

    // Read the file line
    procmetrix_error_t status = PROCMETRIX_ERROR_NONE;
    if (fgets(buf, len, read_file) == NULL)
        status = PROCMETRIX_ERROR_MALFORMED;
    else
    {
        // Remove trailing newline
        buf[strcspn(buf, "\n")] = '\0';
    }

    // Cleanup
    fclose(read_file);

    return status;
}

static procmetrix_error_t procmetrix_impl_linux_proc_pid_read_zero_terminated_tokens(const char *filename, procmetrix_pid_t pid, char ***tokens, size_t *num_tokens)
{
    // Sanity (1)
    if (NULL == tokens || NULL == num_tokens)
        return PROCMETRIX_ERROR_INVALID_ARGUMENT;

    // Cleanup outputs
    *tokens = NULL;
    *num_tokens = 0;

    // Sanity (2)
    if (NULL == filename || 0 == pid)
        return PROCMETRIX_ERROR_INVALID_ARGUMENT;

    // Open the file
    FILE *read_file = procmetrix_impl_linux_proc_pid_open_file(filename, pid);
    if (NULL == read_file)
        return PROCMETRIX_ERROR_FILE_READ;

    // Read the file
    procmetrix_error_t status = PROCMETRIX_ERROR_NONE;

    size_t read_bytes = 0;
    size_t file_size = 0;
    size_t capacity = 128;
    char *buffer = (char *)malloc(capacity);

    if (NULL == buffer)
        status = PROCMETRIX_ERROR_OUT_OF_MEMORY;
    else
    {
        while ((read_bytes = fread(buffer + file_size, 1, capacity - file_size, read_file)) > 0)
        {
            file_size += read_bytes;

            // Grow buffer capacity geometrically
            if (file_size == capacity)
            {
                capacity *= 2;

                // Check rellocation success
                char *new_buffer = (char *)realloc(buffer, capacity);
                if (NULL == new_buffer)
                {
                    status = PROCMETRIX_ERROR_OUT_OF_MEMORY;
                    free(buffer);
                    buffer = NULL;
                    file_size = 0;
                    break;
                }

                // Continue
                buffer = new_buffer;
            }
        }

        // Check if the loop exited due to error
        if (ferror(read_file))
            status = PROCMETRIX_ERROR_FILE_READ;
    }

    // Close the file
    fclose(read_file);
    read_file = NULL;

    // Count how many null terminations
    if (PROCMETRIX_ERROR_NONE == status)
    {
        for (size_t i = 0; i < file_size; ++i)
        {
            if (buffer[i] == '\0' || i == file_size - 1)
                (*num_tokens)++;
        }

        // Allocate a the output list
        if (*num_tokens > 0)
        {
            *tokens = (char **)calloc(*num_tokens, sizeof(char *));
            if (NULL == *tokens)
            {
                status = PROCMETRIX_ERROR_OUT_OF_MEMORY;
                *num_tokens = 0;
            }
            else
            {
                // Fill out the list of tokens
                for (size_t file_idx = 0, tok_idx = 0, start_idx = 0; file_idx < file_size; ++file_idx)
                {
                    int is_last = 0;
                    if (buffer[file_idx] == '\0' || (is_last = (file_idx == file_size - 1)))
                    {
                        (*tokens)[tok_idx++] = strndup(buffer + start_idx, file_idx - start_idx + is_last);
                        start_idx = file_idx + 1;
                    }
                }
            }
        }
    }

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
    if (NULL == stat_data || NULL == ppid)
        return PROCMETRIX_ERROR_INVALID_ARGUMENT;

    // Find delemiter
    const char *rparen = strrchr(stat_data, ')');
    if (NULL == rparen)
        return PROCMETRIX_ERROR_MALFORMED;

    if (sscanf(rparen + 1, " %*c %" SCNu32, ppid) != 1)
        return PROCMETRIX_ERROR_MALFORMED;

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
    glob_t g;
    if (0 != glob("/proc/[0-9]*", 0, NULL, &g))
        return PROCMETRIX_ERROR_FILE_READ;

    // Allocate output list
    procmetrix_pid_t *pids = (procmetrix_pid_t *)calloc(g.gl_pathc, sizeof(procmetrix_pid_t));
    if (NULL == (*list = pids))
    {
        globfree(&g);
        return PROCMETRIX_ERROR_OUT_OF_MEMORY;
    }

    // Fill the list
    procmetrix_error_t status = PROCMETRIX_ERROR_NONE;
    for (*out_count = 0; *out_count < g.gl_pathc; ++(*out_count))
    {
        if (1 != sscanf(g.gl_pathv[*out_count], "/proc/%" SCNu32, &pids[*out_count]))
            status = PROCMETRIX_ERROR_MALFORMED;
    }

    // Cleanup
    globfree(&g);

    return status;
}

procmetrix_error_t procmetrix_get_proc_parent_pid(procmetrix_pid_t pid, procmetrix_pid_t *ppid)
{
    // Sanity
    if (0 == pid || NULL == ppid)
        return PROCMETRIX_ERROR_INVALID_ARGUMENT;

    *ppid = 0;

    // Read the input file
    char buf[4096];
    procmetrix_error_t status = procmetrix_impl_linux_proc_pid_read_line("stat", pid, buf, sizeof(buf));
    if (PROCMETRIX_ERROR_NONE != status)
        return status;

    // Invoke impl
    status = procmetrix_impl_linux_proc_pid_stat_read_ppid(buf, ppid);

    return status;
}

procmetrix_error_t procmetrix_get_proc_name(procmetrix_pid_t pid, char *name, size_t len)
{
    return procmetrix_impl_linux_proc_pid_read_line("comm", pid, name, len);
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
    if (NULL == cmdline)
        return PROCMETRIX_ERROR_INVALID_ARGUMENT;

    return procmetrix_impl_linux_proc_pid_read_zero_terminated_tokens(
        "cmdline", pid, &(cmdline->argv), &(cmdline->argc));
}
