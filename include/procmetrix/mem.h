//! @file
//! @ingroup mem
//! @brief Functions to retrieve memory information.

#ifndef PROCMETRIX_MEM_H
#define PROCMETRIX_MEM_H

// Lib
#include <procmetrix/error.h>

// STD
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus
    //! @addtogroup mem
    //! @{

    //! Statistics about system memory usage
    typedef struct procmetrix_virtual_memory
    {
        //! Total physical memory, excluding swap.
        uint64_t total;

        //! The memory that can be given instantly to processes
        //! without the system going into swap.
        //! This is calculated by summing different memory metrics
        //! that vary depending on the platform.
        uint64_t available;

        //! Memory used, calculated differently depending on the platform.
        //! does not necessarily match total - free.
        uint64_t used;

        //! Memory not being used at all (zeroed).
        //! Note that this doesn’t reflect the actual memory available.
        uint64_t free;

        //! Memory currently in use or very recently used.
        uint64_t active;

        //! Memory that is marked as not used.
        uint64_t inactive;

        //! cache for things like file system metadata.
        //! (Linux, BSD)
        uint64_t buffers;

        //! Cache for various things.
        //! (Linux, BSD)
        uint64_t cached;

        //! Memory that may be simultaneously accessed by multiple processes.
        //! (Linux, BSD)
        uint64_t shared;

        //! In-kernel data structures cache.
        //! (Linux, BSD)
        uint64_t slab;

        //! Usage ratio caculated as 1 - (available / total).
        double ratio;
    } procmetrix_virtual_memory_t;

    //! Return statistics about system memory usage in bytes.
    //! @param[out] virtual_memory Pointer where to store the  read metrics.
    PROCMETRIX_API procmetrix_error_t procmetrix_system_virtual_memory(
        procmetrix_virtual_memory_t* virtual_memory);

    //! System swap memory statistics
    typedef struct procmetrix_swap_memory
    {
        //! Total swap memory.
        uint64_t total;

        //! Used swap memory.
        uint64_t used;

        //! Free swap memory.
        uint64_t free;

        //! Cumulative bytes swapped in from disk.
        uint64_t swap_in;

        //! Cumulative bytes swapped out from disk.
        uint64_t swap_out;

        //! Usage ratio caculated as 1 - (available / total).
        double ratio;
    } procmetrix_swap_memory_t;

    //! Return statistics about system swap memory in bytes.
    //! @param[out] swap_memory Pointer where to store the  retrieved metrics.
    PROCMETRIX_API procmetrix_error_t
    procmetrix_system_swap_memory(procmetrix_swap_memory_t* swap_memory);

    // clang-format off
    //! @}
    // clang-format on
#ifdef __cplusplus
}
#endif // __cplusplus

#endif // PROCMETRIX_MEM_H
