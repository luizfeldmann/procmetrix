// Lib
#include "procmetrix/error.h"
#include <internal/freebsd/common_freebsd_internal.h>

// STD
#include <errno.h>
#include <stdlib.h>

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
procmetrix_impl_bsd_sysctlbyname(const char* name, void* buf, size_t buflen)
{
    // Sanity
    if (NULL == name || NULL == buf || 0 == buflen)
        return PROCMETRIX_ERROR_INVALID_ARGUMENT;

    // Read the data
    size_t readlen = buflen;
    if (0 != sysctlbyname(name, buf, &readlen, NULL, 0))
        return procmetrix_error_from_errno(errno);

    // Check the expected size was read
    // Could mean the expected name and type dont match
    if (readlen > buflen)
        return PROCMETRIX_ERROR_INVALID_ARGUMENT;

    return PROCMETRIX_ERROR_NONE;
}

procmetrix_error_t procmetrix_impl_bsd_sysctl(
    int* mib, unsigned int miblen, void* buf, size_t buflen)
{
    // Sanity
    if (NULL == mib || 0 == miblen || NULL == buf || 0 == buflen)
        return PROCMETRIX_ERROR_INVALID_ARGUMENT;

    // Read the data
    size_t readlen = buflen;
    if (0 != sysctl(mib, miblen, buf, &readlen, NULL, 0))
        return procmetrix_error_from_errno(errno);

    // Check the expected size was read
    if (readlen > buflen)
        return PROCMETRIX_ERROR_INVALID_ARGUMENT;

    return PROCMETRIX_ERROR_NONE;
}

procmetrix_error_t procmetrix_impl_bsd_sysctl_alloc(
    int* mib, unsigned int miblen, void** buf, size_t* buflen)
{
    // Sanity
    if (NULL == buf || NULL == buflen)
        return PROCMETRIX_ERROR_INVALID_ARGUMENT;

    *buf = NULL;
    *buflen = 0;

    if (NULL == mib || 0 == miblen)
        return PROCMETRIX_ERROR_INVALID_ARGUMENT;

    // There is an inherent race between
    // getting the table size and reading its content
    size_t attempts = 10;
    size_t capacity = 0;
    void* data = NULL;

    do
    {
        // Estiamte the size needed for the data
        size_t size_needed = 0;
        if (0 != sysctl(mib, miblen, NULL, &size_needed, NULL, 0))
        {
            free(data);
            return procmetrix_error_from_errno(errno);
        }

        // Add some margin to reduce likelyhood of ENOMEM on busy systems
        size_needed += size_needed / 8;

        // Resize the buffer to fit the newly required data
        if (size_needed > capacity)
        {
            void* new_data = realloc(data, size_needed);
            if (NULL == new_data)
            {
                free(data);
                return PROCMETRIX_ERROR_OUT_OF_MEMORY;
            }

            data = new_data;
            capacity = size_needed;
        }

        // Read the contents
        if (0 == sysctl(mib, miblen, data, &size_needed, NULL, 0))
        {
            // Success
            *buf = data;
            *buflen = size_needed;
            return PROCMETRIX_ERROR_NONE;
        }

        // Can retry if error is due to required size growing
        // For other errors, cannot recover
        if (ENOMEM != errno)
        {
            free(data);
            return procmetrix_error_from_errno(errno);
        }
    } while (--attempts != 0);

    // Exhausted all attempts
    // Cleanup
    free(data);
    return PROCMETRIX_ERROR_UNKNOWN;
}
