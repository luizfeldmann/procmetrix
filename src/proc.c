// Lib
#include <procmetrix/proc.h>

// STD
#include <stdlib.h>

void procmetrix_free_pids(procmetrix_pid_t **list)
{
    if (NULL == list || NULL == *list)
        return;

    free(*list);
    *list = NULL;
}
