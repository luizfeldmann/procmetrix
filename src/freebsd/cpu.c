// Lib
#include "procmetrix/error.h"
#include <procmetrix/cpu.h>

// Internal
#include <internal/freebsd/common_freebsd_internal.h>

// STD
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// System
#include <sys/resource.h>
#include <sys/sysctl.h>

// Utils

static void procmetrix_impl_bsd_transform_cpu_times(
    const long* cpu_states, procmetrix_cpu_times_t* cpu_times)
{
    cpu_times->user = (double)cpu_states[CP_USER] / CLOCKS_PER_SEC;
    cpu_times->nice = (double)cpu_states[CP_NICE] / CLOCKS_PER_SEC;
    cpu_times->system = (double)cpu_states[CP_SYS] / CLOCKS_PER_SEC;
    cpu_times->idle = (double)cpu_states[CP_IDLE] / CLOCKS_PER_SEC;
    cpu_times->irq = (double)cpu_states[CP_INTR] / CLOCKS_PER_SEC;
}

static procmetrix_error_t
procmetrix_impl_bsd_read_cpu_freq(size_t core, procmetrix_cpu_freq_t* cpu_freq)
{
    // Format the path to read
    char oid[64] = { 0 };
    snprintf(oid, sizeof(oid), "dev.cpu.%zu.freq", core);

    int current = 0;
    procmetrix_error_t status =
        procmetrix_impl_bsd_sysctlbyname(oid, &current, sizeof(current));

    if (PROCMETRIX_ERROR_NONE != status)
        return status;

    // Already in MHz
    // https://www.unix.com/man-page/FreeBSD/4/cpufreq/
    cpu_freq->freq_cur = current;
    cpu_freq->freq_min = current;
    cpu_freq->freq_max = current;

    // Try to read freq levels
    char available_freq_levels[1024] = { 0 };
    snprintf(oid, sizeof(oid), "dev.cpu.%zu.freq_levels", core);

    if (PROCMETRIX_ERROR_NONE ==
        procmetrix_impl_bsd_sysctlbyname(
            oid, available_freq_levels, sizeof(available_freq_levels)))
    {
        // Tokenize the list of frequency/power
        char* saveptr = NULL;
        char* tok = strtok_r(available_freq_levels, " ", &saveptr);
        while (tok)
        {
            // Parse the entry
            unsigned freq = 0;
            unsigned power = 0;
            if (sscanf(tok, "%u/%u", &freq, &power) != 2)
                break;
            tok = strtok_r(NULL, " ", &saveptr);

            if (freq > cpu_freq->freq_max)
                cpu_freq->freq_max = freq;

            if (freq < cpu_freq->freq_min)
                cpu_freq->freq_min = freq;
        }
    }

    // Frequency range is optional, no errors if failed to read range
    return PROCMETRIX_ERROR_NONE;
}

static procmetrix_error_t
procmetrix_impl_bsd_read_clockrate(procmetrix_cpu_freq_t* cpu_freq)
{
    int clockrate = 0;
    procmetrix_error_t status = procmetrix_impl_bsd_sysctlbyname(
        "hw.clockrate", &clockrate, sizeof(clockrate));

    if (PROCMETRIX_ERROR_NONE == status)
    {
        cpu_freq->freq_cur = clockrate;
        cpu_freq->freq_min = clockrate;
        cpu_freq->freq_max = clockrate;
    }

    return status;
}

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
    // Sanity
    if (NULL == cpu_times)
        return PROCMETRIX_ERROR_INVALID_ARGUMENT;
    memset(cpu_times, 0, sizeof(*cpu_times));

    // Read cpu times
    long cpu_states[CPUSTATES];

    procmetrix_error_t status = procmetrix_impl_bsd_sysctlbyname(
        "kern.cp_time", &cpu_states, sizeof(cpu_states));
    if (PROCMETRIX_ERROR_NONE != status)
        return status;

    // Convert ticks to seconds
    procmetrix_impl_bsd_transform_cpu_times(cpu_states, cpu_times);

    return status;
}

procmetrix_error_t procmetrix_cpu_times_per_cpu(
    procmetrix_cpu_times_t* cpu_times, size_t max_count, size_t* read_count)
{
    // Defensive cleaning
    if (NULL != read_count)
        *read_count = 0;

    // Sanity
    if (NULL == cpu_times)
        return PROCMETRIX_ERROR_INVALID_ARGUMENT;

    if (0 == max_count)
        return PROCMETRIX_ERROR_MORE_DATA;

    memset(cpu_times, 0, max_count * sizeof(procmetrix_cpu_times_t));

    // Get the number of cpus
    size_t ncpu = procmetrix_cpu_count_logical();
    if (0 == ncpu)
        return PROCMETRIX_ERROR_UNKNOWN;

    // Allocate one item per each cpu
    long(*states)[CPUSTATES] = calloc(ncpu, sizeof(*states));
    if (NULL == states)
        return PROCMETRIX_ERROR_OUT_OF_MEMORY;

    // Read all the data
    size_t query_size = ncpu * sizeof(*states);
    procmetrix_error_t status =
        procmetrix_impl_bsd_sysctlbyname("kern.cp_times", states, query_size);

    // Copy to output
    if (PROCMETRIX_ERROR_NONE == status)
    {
        for (size_t i = 0; i < ncpu; ++i)
        {
            // Limit by output buffer size
            if (i >= max_count)
            {
                status = PROCMETRIX_ERROR_MORE_DATA;
                break;
            }

            procmetrix_impl_bsd_transform_cpu_times(states[i], &cpu_times[i]);

            // Count number of actually read items
            if (NULL != read_count)
                *read_count = i + 1;
        }
    }

    // Cleanup
    free(states);

    return status;
}

procmetrix_error_t procmetrix_cpu_freqs(
    procmetrix_cpu_freq_t* cpu_freqs, size_t max_count, size_t* read_count)
{
    // Defensive cleaning
    if (NULL != read_count)
        *read_count = 0;

    // Sanity
    if (NULL == cpu_freqs || 0 == max_count)
        return PROCMETRIX_ERROR_INVALID_ARGUMENT;

    memset(cpu_freqs, 0, max_count * sizeof(procmetrix_cpu_freq_t));

    // only CPU 0 appears to be supported,
    // and all other cores match the frequency of CPU 0.
    // so lets return a list with a single item.
    procmetrix_error_t status = procmetrix_impl_bsd_read_cpu_freq(0, cpu_freqs);

    // As fallback read clock rate
    if (PROCMETRIX_ERROR_NONE != status)
        status = procmetrix_impl_bsd_read_clockrate(cpu_freqs);

    if (NULL != read_count && PROCMETRIX_ERROR_NONE == status)
        *read_count = 1;

    return status;
}
