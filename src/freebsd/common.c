// Lib
#include "procmetrix/error.h"
#include <internal/freebsd/common_freebsd_internal.h>

// STD
#include <errno.h>

// System
#include <sys/sysctl.h>

// Impl

static procmetrix_error_t procmetrix_error_from_errno(int err)
{
    if (ENOENT == err || EINVAL == err)
        return PROCMETRIX_ERROR_INVALID_ARGUMENT;

    if (ENOMEM == err)
        return PROCMETRIX_ERROR_OUT_OF_MEMORY;

    return PROCMETRIX_ERROR_UNKNOWN;
}

procmetrix_error_t
procmetrix_impl_bsd_sysctlbyname(const char* name, void* buf, size_t len)
{
    // Sanity
    if (NULL == name || NULL == buf || 0 == len)
        return PROCMETRIX_ERROR_INVALID_ARGUMENT;

    // Read the data
    size_t oldlenp = len;
    if (0 != sysctlbyname(name, buf, &oldlenp, NULL, 0))
        return procmetrix_error_from_errno(errno);

    // Check the expected size was read
    // Could mean the expected name and type dont match
    if (oldlenp != len)
        return PROCMETRIX_ERROR_INVALID_ARGUMENT;

    return PROCMETRIX_ERROR_NONE;
}

procmetrix_error_t
procmetrix_impl_bsd_sysctl(int* mib, unsigned int miblen, void* buf, size_t len)
{
    // Sanity
    if (NULL == mib || 0 == miblen || NULL == buf || 0 == len)
        return PROCMETRIX_ERROR_INVALID_ARGUMENT;

    // Read the data
    size_t oldlenp = len;
    if (0 != sysctl(mib, miblen, buf, &len, NULL, 0))
        return procmetrix_error_from_errno(errno);

    // Check the expected size was read
    if (oldlenp != len)
        return PROCMETRIX_ERROR_INVALID_ARGUMENT;

    return PROCMETRIX_ERROR_NONE;
}
