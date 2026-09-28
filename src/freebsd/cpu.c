// Lib
#include "procmetrix/error.h"
#include <procmetrix/cpu.h>

// Internal
#include <internal/freebsd/common_freebsd_internal.h>

// System
#include <sys/sysctl.h>

// Impl

size_t procmetrix_cpu_count_physical(void)
{
    int ncores = 0;
    if (PROCMETRIX_ERROR_NONE == procmetrix_impl_bsd_sysctlbyname(
                                     "kern.smp.cores", &ncores, sizeof(ncores)))
        return ncores;

    return 0;
}

size_t procmetrix_cpu_count_logical(void)
{
    int mib[] = { CTL_HW, HW_NCPU };

    int ncpu = 0;
    if (PROCMETRIX_ERROR_NONE ==
        procmetrix_impl_bsd_sysctl(mib, 2, &ncpu, sizeof(ncpu)))
        return ncpu;

    return 0;
}

procmetrix_error_t procmetrix_cpu_times_total(procmetrix_cpu_times_t* cpu_times)
{
    return PROCMETRIX_NOT_IMPLEMENTED;
}

procmetrix_error_t procmetrix_cpu_times_per_cpu(
    procmetrix_cpu_times_t* cpu_times, size_t max_count, size_t* read_count)
{
    return PROCMETRIX_NOT_IMPLEMENTED;
}

procmetrix_error_t procmetrix_cpu_freqs(
    procmetrix_cpu_freq_t* cpu_freqs, size_t max_count, size_t* read_count)
{
    return PROCMETRIX_NOT_IMPLEMENTED;
}
