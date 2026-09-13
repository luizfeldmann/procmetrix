//! @file
//! @ingroup proc
//! @brief Functions to retrieve process information.

#ifndef _PROCMETRIX_PROC_H_
#define _PROCMETRIX_PROC_H_

// Lib
#include <procmetrix/error.h>

// STD
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus
    //! @addtogroup proc
    //! @{

    //! Type of process ID
    typedef uint32_t procmetrix_pid_t;

    //! Gets the ID of the current process
    procmetrix_pid_t procmetrix_get_pid();

    //! Checks if a process ID exists.
    bool procmetrix_pid_exists(procmetrix_pid_t);

    //! Allocates and fills a list of running PIDs.
    //! @details Caller must later free the list with procmetrix_free_pids.
    //! @param[out] list Pointer to the memory location where the output array will be allocated.
    //                   The address of the variable pointed by list must be initialized to NULL before the call.
    //! @param[out] out_count Pointer to the variable which will receive the number of returned items
    procmetrix_error_t procmetrix_list_pids(procmetrix_pid_t **list, size_t *out_count);

    //! Frees the list allocated with procmetrix_list_pids.
    //! @details Safe to call with NULL.
    void procmetrix_free_pids(procmetrix_pid_t **list);

    //! @}
#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _PROCMETRIX_PROC_H_
