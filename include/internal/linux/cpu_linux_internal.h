#ifndef _PROCMETRIX_CPU_LINUX_INTERNAL_H_
#define _PROCMETRIX_CPU_LINUX_INTERNAL_H_

// Lib
#include <procmetrix/cpu.h>

// STD
#include <stdio.h>

// Linux
#include <glob.h>

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

    //! Counts the number of logical processors by reading /proc/cpuinfo
    //! @private
    PROCMETRIX_API size_t procmetrix_impl_linux_cpuinfo_count_processors(FILE *cpuinfo_file);

    //! Counts the number of logical processors by reading /proc/stat
    //! @private
    PROCMETRIX_API size_t procmetrix_impl_linux_procstat_count_cpus(FILE *stat_file);

    //! Counts the number of physical processors by reading /proc/cpuinfo
    //! @private
    PROCMETRIX_API size_t procmetrix_impl_linux_cpu_count_physical_cpuinfo(FILE *cpuinfo_file);

    //! Counts the number of physical processors by reading /sys/devices/system/cpu/cpu[0-9]*/topology
    //! @private
    PROCMETRIX_API size_t procmetrix_impl_linux_cpu_count_physical_topology(const glob_t *sysfs_cpus);

    //! Reads aggregate cpu times from /stat/proc
    //! @private
    PROCMETRIX_API procmetrix_error_t procmetrix_impl_linux_cpu_times_total(FILE *stat_file, procmetrix_cpu_times_t *cpu_times);

    //! Reads individual cpu times from /stat/proc
    //! @private
    PROCMETRIX_API procmetrix_error_t procmetrix_impl_linux_cpu_times_per_cpu(FILE *stat_file, procmetrix_cpu_times_t *cpu_times, size_t max_count, size_t *read_count);

    //! Reads CPU frequencies from /sys/devices/system/cpu/cpufreq/policyX
    PROCMETRIX_API procmetrix_error_t procmetrix_impl_linux_cpu_freqs_policies(const glob_t *sysfs_policies, procmetrix_cpu_freq_t *cpu_freqs, size_t max_count, size_t *read_count);

    //! Reads the CPU frequencies from /stat/cpuinfo
    PROCMETRIX_API procmetrix_error_t procmetrix_impl_linux_cpuinfo_freqs(FILE *cpuinfo_file, procmetrix_cpu_freq_t *cpu_freqs, size_t max_count, size_t *read_count);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _PROCMETRIX_CPU_LINUX_INTERNAL_H_
