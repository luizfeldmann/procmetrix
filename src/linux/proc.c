// Lib
#include <procmetrix/proc.h>

// STD
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <inttypes.h>

// Linux
#include <glob.h>
#include <signal.h>
#include <unistd.h>

// Impl

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
