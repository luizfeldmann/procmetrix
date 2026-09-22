// Lib
#include <procmetrix/cpu.h>

// Windows
#include <Windows.h>
#include <winternl.h>
#include <powrprof.h>
#pragma comment(lib, "ntdll.lib")
#pragma comment(lib, "PowrProf.lib")

// Util

inline static double large_uint_to_secs(const ULARGE_INTEGER *li)
{
    // each unit is 100 nanoseconds
    return (double)li->QuadPart / 1.0E7;
}

inline static double large_int_to_secs(const LARGE_INTEGER *li)
{
    // each unit is 100 nanoseconds
    return (double)li->QuadPart / 1.0E7;
}

inline static double filetime_to_secs(const FILETIME *pft)
{
    ULARGE_INTEGER v;
    v.LowPart = pft->dwLowDateTime;
    v.HighPart = pft->dwHighDateTime;

    // each unit is 100 nanoseconds
    return large_uint_to_secs(&v);
}

//! This seems to be missing from windows headers
typedef struct _PROCESSOR_POWER_INFORMATION
{
    ULONG Number;
    ULONG MaxMhz;
    ULONG CurrentMhz;
    ULONG MhzLimit;
    ULONG MaxIdleState;
    ULONG CurrentIdleState;
} PROCESSOR_POWER_INFORMATION;

// Impl

size_t procmetrix_cpu_count_physical(void)
{
    // Discover required buffer size
    DWORD len = 0;
    GetLogicalProcessorInformationEx(RelationProcessorCore, NULL, &len);
    if (ERROR_INSUFFICIENT_BUFFER != GetLastError())
        return 0;

    // Allocate required size and read the info
    PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX buf = malloc(len);
    if (NULL == buf)
        return PROCMETRIX_ERROR_OUT_OF_MEMORY;

    if (!GetLogicalProcessorInformationEx(RelationProcessorCore, buf, &len))
    {
        free(buf);
        return 0;
    }

    // Iterate the collection
    size_t physicalCores = 0;
    BYTE *ptr = (BYTE *)buf;
    BYTE *end = ptr + len;
    while (ptr < end)
    {
        PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX info =
            (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX)ptr;

        if (info->Relationship == RelationProcessorCore)
            physicalCores++;

        // Iteration step is a variable length
        ptr += info->Size;
    }

    // Cleanup
    free(buf);

    return physicalCores;
}

size_t procmetrix_cpu_count_logical(void)
{
    return GetActiveProcessorCount(ALL_PROCESSOR_GROUPS);
}

procmetrix_error_t procmetrix_cpu_times_total(procmetrix_cpu_times_t *cpu_times)
{
    // Sanity
    if (NULL == cpu_times)
        return PROCMETRIX_ERROR_INVALID_ARGUMENT;
    memset(cpu_times, 0, sizeof(*cpu_times));

    // Query from OS
    FILETIME idle_time, kernel_time, user_time;
    if (!GetSystemTimes(&idle_time, &kernel_time, &user_time))
        return PROCMETRIX_ERROR_UNKNOWN;

    // Convert to seconds
    cpu_times->idle = filetime_to_secs(&idle_time);
    cpu_times->user = filetime_to_secs(&user_time);

    // Kernel time includes idle time.
    // We return only busy kernel time subtracting idle time
    double kernel = filetime_to_secs(&kernel_time);
    cpu_times->system = kernel - cpu_times->idle;

    return PROCMETRIX_ERROR_NONE;
}

procmetrix_error_t procmetrix_cpu_times_per_cpu(procmetrix_cpu_times_t *cpu_times, size_t maxCount, size_t *readCount)
{
    // Defensive cleaning
    if (NULL != readCount)
        *readCount = 0;

    // Sanity
    if (NULL == cpu_times)
        return PROCMETRIX_ERROR_INVALID_ARGUMENT;
    memset(cpu_times, 0, maxCount * sizeof(*cpu_times));

    if (0 == maxCount)
        return PROCMETRIX_ERROR_MORE_DATA;

    // Allocate one item per CPU
    size_t ncpus = procmetrix_cpu_count_logical();
    if (0 == ncpus)
        return PROCMETRIX_ERROR_UNKNOWN;

    SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION *sppi =
        (SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION *)calloc(ncpus, sizeof(SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION));

    if (NULL == sppi)
        return PROCMETRIX_ERROR_OUT_OF_MEMORY;

    // Get CPU times information
    procmetrix_error_t status = PROCMETRIX_ERROR_NONE;

    ULONG retlen = 0;
    if (!NT_SUCCESS(NtQuerySystemInformation(
            SystemProcessorPerformanceInformation, sppi, ncpus * sizeof(SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION), &retlen)))
    {
        status = PROCMETRIX_ERROR_UNKNOWN;
    }
    else
    {
        // The kernel may return entries for less CPUs
        // on systems with more than 64 CPUs it only covers the calling thread's processor group
        ncpus = retlen / sizeof(SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION);

        for (size_t i = 0; i < ncpus; i++)
        {
            // Limit by the user output buffer size
            if (i >= maxCount)
            {
                status = PROCMETRIX_ERROR_MORE_DATA;
                break;
            }

            // Convert to seconds
            cpu_times[i].user = large_int_to_secs(&sppi[i].UserTime);
            cpu_times[i].idle = large_int_to_secs(&sppi[i].IdleTime);

            // kernel time includes idle time on windows
            // we return only system busy kernel time subtracting the idle time from the kernel total time
            double kernel_time = large_int_to_secs(&sppi[i].KernelTime);
            cpu_times[i].system = kernel_time - cpu_times[i].idle;

            cpu_times[i].dpc = large_int_to_secs(&sppi[i].Reserved1[0]);
            cpu_times[i].interrupt = large_int_to_secs(&sppi[i].Reserved1[1]);

            // Update count of read items
            if (NULL != readCount)
                *readCount = i + 1;
        }
    }

    // Cleanup
    free(sppi);

    return status;
}

procmetrix_error_t procmetrix_cpu_freqs(procmetrix_cpu_freq_t *cpu_freqs, size_t max_count, size_t *read_count)
{
    // Defensive cleaning
    if (NULL != read_count)
        *read_count = 0;

    // Sanity
    if (NULL == cpu_freqs)
        return PROCMETRIX_ERROR_INVALID_ARGUMENT;
    memset(cpu_freqs, 0, max_count * sizeof(*cpu_freqs));

    if (0 == max_count)
        return PROCMETRIX_ERROR_INVALID_ARGUMENT;

    // Allocate one item per CPU
    DWORD ncpus = procmetrix_cpu_count_logical();
    if (0 == ncpus)
        return PROCMETRIX_ERROR_UNKNOWN;

    PROCESSOR_POWER_INFORMATION *ppi = (PROCESSOR_POWER_INFORMATION *)calloc(ncpus, sizeof(PROCESSOR_POWER_INFORMATION));
    if (NULL == ppi)
        return PROCMETRIX_ERROR_OUT_OF_MEMORY;

    // Get processor power informatioon
    procmetrix_error_t status = PROCMETRIX_ERROR_NONE;

    if (ERROR_SUCCESS != CallNtPowerInformation(
                             ProcessorInformation, NULL, 0, ppi, ncpus * sizeof(*ppi)))
    {
        status = PROCMETRIX_ERROR_INVALID_ARGUMENT;
    }
    else
    {
        for (size_t i = 0; i < ncpus; i++)
        {
            // Limit by the user output buffer size
            if (i >= max_count)
            {
                status = PROCMETRIX_ERROR_MORE_DATA;
                break;
            }

            cpu_freqs[i].freq_cur = ppi[i].CurrentMhz;
            cpu_freqs[i].freq_max = ppi[i].MaxMhz;

            // Update count of read items
            if (NULL != read_count)
                *read_count = i + 1;
        }
    }

    // Cleanup
    free(ppi);

    return status;
}
