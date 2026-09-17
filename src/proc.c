// Lib
#include <procmetrix/proc.h>

// STD
#include <stdlib.h>
#include <string.h>

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

