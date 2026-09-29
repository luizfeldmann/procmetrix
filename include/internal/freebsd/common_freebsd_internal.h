#ifndef PROCMETRIX_COMMON_FREEBSD_INTERNAL_H
#define PROCMETRIX_COMMON_FREEBSD_INTERNAL_H

// Lib
#include <procmetrix/error.h>

// System
#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

    //! A thin wrapper around sysctlbyname
    //! @private
    procmetrix_error_t
    procmetrix_impl_bsd_sysctlbyname(const char* name, void* buf, size_t len);

    //! A thin wrapper around sysctl
    //! @private
    procmetrix_error_t procmetrix_impl_bsd_sysctl(
        int* mib, unsigned int miblen, void* buf, size_t len);

    //! Allocates a buffer containing the result of a sysctl
    //! @private
    procmetrix_error_t procmetrix_impl_bsd_sysctl_alloc(
        int* mib, unsigned int miblen, void** buf, size_t* buflen);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // PROCMETRIX_COMMON_FREEBSD_INTERNAL_H
