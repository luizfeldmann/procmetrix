// Lib
#include <procmetrix/mem.h>

// Windows
#include <Psapi.h>
#include <Windows.h>
#include <pdh.h>
#pragma comment(lib, "pdh.lib")

// Utils

static procmetrix_error_t swap_used_ratio(double* percent_swap_usage)
{
    procmetrix_error_t status = PROCMETRIX_ERROR_UNKNOWN;

    PDH_HQUERY query = NULL;
    if (ERROR_SUCCESS == PdhOpenQuery(NULL, 0, &query))
    {
        PDH_HCOUNTER counter = NULL;
        if (ERROR_SUCCESS ==
            PdhAddEnglishCounterA(
                query, "\\Paging File(_Total)\\% Usage", 0, &counter))
        {
            PDH_FMT_COUNTERVALUE counter_value;
            if (ERROR_SUCCESS == PdhCollectQueryData(query) &&
                ERROR_SUCCESS == PdhGetFormattedCounterValue(
                                     (PDH_HCOUNTER)counter,
                                     PDH_FMT_DOUBLE,
                                     0,
                                     &counter_value))
            {
                // Convert from percent to ratio
                *percent_swap_usage = counter_value.doubleValue / 100.0;
                status = PROCMETRIX_ERROR_NONE;
            }

            PdhRemoveCounter(counter);
        }

        PdhCloseQuery(query);
    }

    return status;
}

// Impl

procmetrix_error_t
procmetrix_system_virtual_memory(procmetrix_virtual_memory_t* virtual_memory)
{
    // Sanity
    if (NULL == virtual_memory)
        return PROCMETRIX_ERROR_INVALID_ARGUMENT;

    // Consistent results regardless of errors
    memset(virtual_memory, 0, sizeof(*virtual_memory));

    // Read from OS
    PERFORMANCE_INFORMATION info;
    if (!GetPerformanceInfo(&info, sizeof(PERFORMANCE_INFORMATION)))
        return PROCMETRIX_ERROR_UNKNOWN;

    size_t pagesize = info.PageSize;
    virtual_memory->total = pagesize * info.PhysicalTotal;
    virtual_memory->available = pagesize * info.PhysicalAvailable;
    virtual_memory->cached = pagesize * info.SystemCache;

    // Derived metrics
    virtual_memory->free = virtual_memory->available;
    virtual_memory->used = virtual_memory->total - virtual_memory->available;
    virtual_memory->ratio =
        (double)virtual_memory->used / (double)virtual_memory->total;

    return PROCMETRIX_ERROR_NONE;
}

procmetrix_error_t
procmetrix_system_swap_memory(procmetrix_swap_memory_t* swap_memory)
{
    // Sanity
    if (NULL == swap_memory)
        return PROCMETRIX_ERROR_INVALID_ARGUMENT;

    // Consistent results regardless of errors
    memset(swap_memory, 0, sizeof(*swap_memory));

    // Read total swap OS
    PERFORMANCE_INFORMATION info;
    if (!GetPerformanceInfo(&info, sizeof(PERFORMANCE_INFORMATION)))
        return PROCMETRIX_ERROR_UNKNOWN;

    // Read used swap ratio
    double percent_swap_usage = 0.0;
    procmetrix_error_t status = swap_used_ratio(&percent_swap_usage);

    if (PROCMETRIX_ERROR_NONE == status)
    {
        size_t pagesize = info.PageSize;
        swap_memory->total = pagesize * (info.CommitLimit - info.PhysicalTotal);
        swap_memory->used = percent_swap_usage * swap_memory->total;
        swap_memory->free = swap_memory->total - swap_memory->used;

        swap_memory->ratio =
            (double)swap_memory->used / (double)swap_memory->total;
    }

    return status;
}
