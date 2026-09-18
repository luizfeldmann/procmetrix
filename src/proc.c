// Lib
#include <procmetrix/proc.h>

// STD
#include <stdlib.h>
#include <string.h>
#include <math.h>

/** Cleanup functions */

void procmetrix_free_pids(procmetrix_pid_t **list)
{
    if (NULL == list || NULL == *list)
        return;

    free(*list);
    *list = NULL;
}

void procmetrix_free_proc_cmdline(procmetrix_proc_cmdline_t *cmd_line)
{
    // Sanity
    if (NULL == cmd_line)
        return;

    // Free each string in the array
    for (size_t i = 0; i < cmd_line->argc; ++i)
        free(cmd_line->argv[i]);

    // Free the array itself
    free(cmd_line->argv);

    // No garbage left in the struct
    memset(cmd_line, 0, sizeof(*cmd_line));
}

void procmetrix_free_proc_environ(procmetrix_proc_environ_t *environ)
{
    // Sanity
    if (NULL == environ)
        return;

    // Free each item in the array
    for (size_t i = 0; i < environ->count; ++i)
    {
        free(environ->vars[i].name);
        free(environ->vars[i].value);
    }

    // Free the array itself
    free(environ->vars);

    // No garbage left in the struct
    memset(environ, 0, sizeof(*environ));
}

/** Derived metrics */

procmetrix_error_t procmetrix_proc_cpu_times_delta(const procmetrix_proc_cpu_times_t *before, const procmetrix_proc_cpu_times_t *after, procmetrix_proc_cpu_times_t *delta)
{
    // Sanity
    if (NULL == delta)
        return PROCMETRIX_ERROR_INVALID_ARGUMENT;

    if (NULL == before || NULL == after)
    {
        memset(delta, 0, sizeof(*delta));
        return PROCMETRIX_ERROR_INVALID_ARGUMENT;
    }

    // Just diff before and after of every field
    delta->user = fmax(0.0, after->user - before->user);
    delta->system = fmax(0.0, after->system - before->system);
    delta->children_user = fmax(0.0, after->children_user - before->children_user);
    delta->children_system = fmax(0.0, after->children_system - before->children_system);

    return PROCMETRIX_ERROR_NONE;
}

double procmetrix_proc_cpu_times_sum(const procmetrix_proc_cpu_times_t *cpu_times)
{
    // Sanity
    if (NULL == cpu_times)
        return 0.0;

    // Just sums user and system times
    return cpu_times->user + cpu_times->system;
}
