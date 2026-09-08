//! @file
//! @ingroup mem
//! @brief Functions to retrieve memory information.

#ifndef _PROCMETRIX_MEM_H_
#define _PROCMETRIX_MEM_H_

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

        //! The memory that can be given instantly to processes without the system going into swap.
        //! This is calculated by summing different memory metrics that vary depending on the platform.
        uint64_t available;

        //! Memory used, calculated differently depending on the platform.
        //! does not necessarily match \f$ total - free \f$.
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

        //! Usage ratio caculated as \f$ 1 - (available / total) \f$.
        double ratio;
    } procmetrix_virtual_memory_t;

    //! Return statistics about system memory usage in bytes.
    //! @param[out] virtual_memory Pointer to struct where the retrieved metrics will be stored.
    PROCMETRIX_API procmetrix_error_t procmetrix_system_virtual_memory(procmetrix_virtual_memory_t *virtual_memory);

    //! @}
#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _PROCMETRIX_MEM_H_
