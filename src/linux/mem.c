// Internal
#include <internal/linux/common_linux_internal.h>
#include <internal/linux/mem_linux_internal.h>

// STD
#include <string.h>
#include <stdbool.h>
#include <inttypes.h>

// Linux
#include <unistd.h>

//! Path to the meminfo file
static const char *g_proc_meminfo_file = "/proc/meminfo";

//! Path to the vmstat file
static const char *g_proc_vmstat_file = "/proc/vmstat";

//! Path to the zoneinfo file
static const char *g_proc_zoneinfo_file = "/proc/zoneinfo";

// Helpers

//! Least of two numbers
static inline uint64_t min_u64(uint64_t a, uint64_t b)
{
    return a < b ? a : b;
}

// Private impl

procmetrix_error_t procmetrix_impl_linux_zoneinfo_low_watermark(FILE *zoneinfo_file, uint64_t *low_wmark)
{
    // Sanity
    if (NULL == low_wmark)
        return PROCMETRIX_ERROR_INVALID_ARGUMENT;

    // Define the error behavior
    *low_wmark = 0;

    // Sanity (2)
    if (NULL == zoneinfo_file)
        return PROCMETRIX_ERROR_INVALID_ARGUMENT;

    long page_size = sysconf(_SC_PAGE_SIZE);
    if (0 >= page_size)
        return PROCMETRIX_ERROR_UNKNOWN;

    // Iterate the file lines
    uint64_t temp_value = 0;

    char line[1024];
    while (NULL != fgets(line, sizeof(line), zoneinfo_file))
        if (1 == sscanf(line, " low %" SCNu64, &temp_value))
            *low_wmark += temp_value;

    // Must multiply by the page size
    *low_wmark *= page_size;

    return PROCMETRIX_ERROR_NONE;
}

procmetrix_error_t procmetrix_impl_linux_system_virtual_memory(FILE *meminfo_file, FILE *zoneinfo_file, procmetrix_virtual_memory_t *virtual_memory)
{
    // Sanity
    if (NULL == virtual_memory)
        return PROCMETRIX_ERROR_INVALID_ARGUMENT;

    // Defensively return consistent result in case of error
    memset(virtual_memory, 0, sizeof(procmetrix_virtual_memory_t));

    if (NULL == meminfo_file)
        return PROCMETRIX_ERROR_INVALID_ARGUMENT;

    // Helper variables
    uint64_t cached = 0,
             reclaimable = 0,
             active_file = 0,
             inactive_file = 0;
    bool has_reclaimable = false,
         has_active_file = false,
         has_inactive_file = false;

    // Iterate the file lines
    char line[1024];
    while (NULL != fgets(line, sizeof(line), meminfo_file))
    {
        uint64_t temp_value = 0;

        if (1 == sscanf(line, "MemTotal: %" SCNu64, &temp_value))
        {
            virtual_memory->total = 1024ull * temp_value;
        }
        else if (1 == sscanf(line, "MemFree: %" SCNu64, &temp_value))
        {
            virtual_memory->free = 1024ull * temp_value;
        }
        else if (1 == sscanf(line, "Buffers: %" SCNu64, &temp_value))
        {
            virtual_memory->buffers = 1024ull * temp_value;
        }
        else if (1 == sscanf(line, "Cached: %" SCNu64, &temp_value))
        {
            cached = 1024ull * temp_value;
        }
        else if (1 == sscanf(line, "SReclaimable: %" SCNu64, &temp_value))
        {
            reclaimable = 1024ull * temp_value;
            has_reclaimable = true;
        }
        else if (1 == sscanf(line, "Shmem: %" SCNu64, &temp_value) ||
                 1 == sscanf(line, "MemShared: %" SCNu64, &temp_value))
        {
            virtual_memory->shared = 1024ull * temp_value;
        }
        else if (1 == sscanf(line, "Active: %" SCNu64, &temp_value))
        {
            virtual_memory->active = 1024ull * temp_value;
        }
        else if (1 == sscanf(line, "Active(file): %" SCNu64, &temp_value))
        {
            active_file = 1024ull * temp_value;
            has_active_file = true;
        }
        else if (1 == sscanf(line, "Inactive(file): %" SCNu64, &temp_value))
        {
            inactive_file = 1024ull * temp_value;
            has_inactive_file = true;
        }
        else if (1 == sscanf(line, "Inactive: %" SCNu64, &temp_value))
        {
            virtual_memory->inactive = 1024ull * temp_value;
        }
        else if (1 == sscanf(line, "Inact_dirty: %" SCNu64, &temp_value) ||
                 1 == sscanf(line, "Inact_clean: %" SCNu64, &temp_value) ||
                 1 == sscanf(line, "Inact_laundry: %" SCNu64, &temp_value))
        {
            // Inactive can be expressed as the sum of subfields
            virtual_memory->inactive += 1024ull * temp_value;
        }
        else if (1 == sscanf(line, "Slab: %" SCNu64, &temp_value))
        {
            virtual_memory->slab = 1024ull * temp_value;
        }
        else if (1 == sscanf(line, "MemAvailable: %" SCNu64, &temp_value))
        {
            virtual_memory->available = 1024ull * temp_value;
        }
    }

    // Both "Cached:" and "SReclaimable:" contribute here
    virtual_memory->cached = cached + reclaimable;

    // Free memory is mandatory to proceed with estimates
    if (0 == virtual_memory->free)
        return PROCMETRIX_ERROR_MALFORMED;

    // Fallback to calculate available
    if (0 == virtual_memory->available)
    {
        // Initial approximation
        virtual_memory->available = virtual_memory->free + cached;

        // A better approximation is possible when these fields exist
        uint64_t low_wmark = 0;
        if (has_reclaimable && has_active_file && has_inactive_file &&
            (PROCMETRIX_ERROR_NONE == procmetrix_impl_linux_zoneinfo_low_watermark(zoneinfo_file, &low_wmark)))
        {
            uint64_t pagecache = active_file + inactive_file;
            pagecache -= min_u64(pagecache / 2, low_wmark);

            virtual_memory->available =
                virtual_memory->free + pagecache + reclaimable - min_u64(reclaimable / 2, low_wmark);

            if (virtual_memory->available > low_wmark)
                virtual_memory->available -= low_wmark;
            else
                virtual_memory->available = 0;
        }
    }

    // Total memory is mandatory to proceed with estimates
    if (0 == virtual_memory->total)
        return PROCMETRIX_ERROR_MALFORMED;

    // If avail is greater than total or our calculation overflows,
    // that's symptomatic of running within a LXC container where such
    // values will be dramatically distorted
    if (virtual_memory->available > virtual_memory->total)
        virtual_memory->available = virtual_memory->free;

    // Calculate usage ration
    virtual_memory->used = virtual_memory->total - virtual_memory->available;
    virtual_memory->ratio = (double)virtual_memory->used / (double)virtual_memory->total;

    // Success
    return PROCMETRIX_ERROR_NONE;
}

// Public impl

procmetrix_error_t procmetrix_system_virtual_memory(procmetrix_virtual_memory_t *virtual_memory)
{
    // Sanity
    if (NULL == virtual_memory)
        return PROCMETRIX_ERROR_INVALID_ARGUMENT;

    // Consistent results regardless of errors
    memset(virtual_memory, 0, sizeof(procmetrix_virtual_memory_t));

    // Open meminfo file
    FILE *meminfo_file = procmetrix_impl_linux_open_file_rdonly_cloexec(g_proc_meminfo_file);
    if (NULL == meminfo_file)
        return PROCMETRIX_ERROR_FILE_READ;

    // Open zoneinfo file (optional)
    FILE *zoneinfo_file = procmetrix_impl_linux_open_file_rdonly_cloexec(g_proc_zoneinfo_file);

    procmetrix_error_t status = procmetrix_impl_linux_system_virtual_memory(meminfo_file, zoneinfo_file, virtual_memory);

    // Cleanup
    fclose(meminfo_file);
    if (NULL != zoneinfo_file)
        fclose(zoneinfo_file);

    return status;
}
