// Lib
#include <procmetrix/cpu.h>

// Windows primary
#include <Windows.h>
// Windows extra
#include <powrprof.h>
#include <winternl.h>
#pragma comment(lib, "ntdll.lib")
#pragma comment(lib, "PowrProf.lib")

// Internal
#include <internal/windows/common_windows_internal.h>
#include <internal/windows/winnt_extended_api.h>

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
    size_t physical_cores = 0;
    BYTE* ptr = (BYTE*)buf;
    BYTE* end = ptr + len;
    while (ptr < end)
    {
        PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX info =
            (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX)ptr;

        if (info->Relationship == RelationProcessorCore)
            physical_cores++;

        // Iteration step is a variable length
        ptr += info->Size;
    }

    // Cleanup
    free(buf);

    return physical_cores;
}

size_t procmetrix_cpu_count_logical(void)
{
    return GetActiveProcessorCount(ALL_PROCESSOR_GROUPS);
}

procmetrix_error_t procmetrix_cpu_times_total(procmetrix_cpu_times_t* cpu_times)
{
    // Sanity
    if (NULL == cpu_times)
        return PROCMETRIX_ERROR_INVALID_ARGUMENT;
    memset(cpu_times, 0, sizeof(*cpu_times));

    // Query from OS
    FILETIME idle_time = { 0 };
    FILETIME kernel_time = { 0 };
    FILETIME user_time = { 0 };
    if (!GetSystemTimes(&idle_time, &kernel_time, &user_time))
        return PROCMETRIX_ERROR_UNKNOWN;

    // Convert to seconds
    cpu_times->idle = procmetrix_impl_windows_filetime_to_secs(&idle_time);
    cpu_times->user = procmetrix_impl_windows_filetime_to_secs(&user_time);

    // Kernel time includes idle time.
    // We return only busy kernel time subtracting idle time
    double kernel = procmetrix_impl_windows_filetime_to_secs(&kernel_time);
    cpu_times->system = kernel - cpu_times->idle;

    return PROCMETRIX_ERROR_NONE;
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
    memset(cpu_times, 0, max_count * sizeof(*cpu_times));

    if (0 == max_count)
        return PROCMETRIX_ERROR_MORE_DATA;

    // Allocate one item per CPU
    size_t ncpus = procmetrix_cpu_count_logical();
    if (0 == ncpus)
        return PROCMETRIX_ERROR_UNKNOWN;

    SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION* sppi =
        (SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION*)calloc(
            ncpus, sizeof(SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION));

    if (NULL == sppi)
        return PROCMETRIX_ERROR_OUT_OF_MEMORY;

    // Get CPU times information
    procmetrix_error_t status = PROCMETRIX_ERROR_NONE;

    ULONG retlen = 0;
    if (!NT_SUCCESS(NtQuerySystemInformation(
            SystemProcessorPerformanceInformation,
            sppi,
            ncpus * sizeof(SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION),
            &retlen)))
    {
        status = PROCMETRIX_ERROR_UNKNOWN;
    }
    else
    {
        // The kernel may return entries for less CPUs
        // on systems with more than 64 CPUs it only covers the calling thread's
        // processor group
        ncpus = retlen / sizeof(SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION);

        for (size_t i = 0; i < ncpus; i++)
        {
            // Limit by the user output buffer size
            if (i >= max_count)
            {
                status = PROCMETRIX_ERROR_MORE_DATA;
                break;
            }

            // Convert to seconds
            cpu_times[i].user =
                procmetrix_impl_windows_large_int_to_secs(&sppi[i].UserTime);
            cpu_times[i].idle =
                procmetrix_impl_windows_large_int_to_secs(&sppi[i].IdleTime);

            // kernel time includes idle time on windows
            // we return only system busy kernel time subtracting the idle time
            // from the kernel total time
            double kernel_time =
                procmetrix_impl_windows_large_int_to_secs(&sppi[i].KernelTime);
            cpu_times[i].system = kernel_time - cpu_times[i].idle;

            cpu_times[i].dpc = procmetrix_impl_windows_large_int_to_secs(
                &sppi[i].Reserved1[0]);
            cpu_times[i].interrupt = procmetrix_impl_windows_large_int_to_secs(
                &sppi[i].Reserved1[1]);

            // Update count of read items
            if (NULL != read_count)
                *read_count = i + 1;
        }
    }

    // Cleanup
    free(sppi);

    return status;
}

procmetrix_error_t procmetrix_cpu_freqs(
    procmetrix_cpu_freq_t* cpu_freqs, size_t max_count, size_t* read_count)
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

    PROCMETRIX_PROCESSOR_POWER_INFORMATION* ppi =
        (PROCMETRIX_PROCESSOR_POWER_INFORMATION*)calloc(
            ncpus, sizeof(PROCMETRIX_PROCESSOR_POWER_INFORMATION));
    if (NULL == ppi)
        return PROCMETRIX_ERROR_OUT_OF_MEMORY;

    // Get processor power informatioon
    procmetrix_error_t status = PROCMETRIX_ERROR_NONE;

    if (ERROR_SUCCESS !=
        CallNtPowerInformation(
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
