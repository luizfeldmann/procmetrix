#ifndef _PROCMETRIX_CPU_LINUX_INTERNAL_H_
#define _PROCMETRIX_CPU_LINUX_INTERNAL_H_

// Lib
#include <procmetrix/cpu.h>

// STD
#include <stdio.h>

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

    //! @private
    PROCMETRIX_API size_t procmetrix_impl_linux_cpuinfo_count_processors(FILE *cpuinfo_file);

    //! @private
    PROCMETRIX_API size_t procmetrix_impl_linux_procstat_count_cpus(FILE *stat_file);

    //! @private
    PROCMETRIX_API procmetrix_error_t procmetrix_impl_linux_cpu_times_total(FILE *stat_file, procmetrix_cpu_times_t *cpu_times);

    //! @private
    PROCMETRIX_API procmetrix_error_t procmetrix_impl_linux_cpu_times_per_cpu(FILE *stat_file, procmetrix_cpu_times_t *cpu_times, size_t max_count, size_t *read_count);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _PROCMETRIX_CPU_LINUX_INTERNAL_H_
