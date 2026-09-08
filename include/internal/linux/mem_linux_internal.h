#ifndef _PROCMETRIX_MEM_LINUX_INTERNAL_H_
#define _PROCMETRIX_MEM_LINUX_INTERNAL_H_

// Lib
#include <procmetrix/mem.h>

// STD
#include <stdio.h>

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

    //! Reads low watermark from /proc/zoneinfo
    PROCMETRIX_API procmetrix_error_t procmetrix_impl_linux_zoneinfo_low_watermark(FILE *zoneinfo_file, uint64_t *low_wmark);

    //! Reads virtual memory from /proc/meminfo
    //! @private
    PROCMETRIX_API procmetrix_error_t procmetrix_impl_linux_system_virtual_memory(FILE *meminfo_file, FILE *zoneinfo_file, procmetrix_virtual_memory_t *virtual_memory);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _PROCMETRIX_MEM_LINUX_INTERNAL_H_
