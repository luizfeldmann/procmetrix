// Lib
#include <procmetrix/proc.h>

// STD
#include <errno.h>
#include <signal.h>
#include <unistd.h>

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
