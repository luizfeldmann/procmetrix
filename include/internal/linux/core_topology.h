#ifndef _PROCMETRIX_LINUX_INTERNAL_CORE_TOPOLOGY_H_
#define _PROCMETRIX_LINUX_INTERNAL_CORE_TOPOLOGY_H_

// Lib
#include <procmetrix/api.h>

// STD
#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

    //! Key for core topology
    //! @private
    typedef struct procmetrix_core_topo_key
    {
        size_t package_id;
        size_t core_id;
    } procmetrix_core_topo_key_t;

    //! List of core topology entries
    //! @private
    typedef struct procmetrix_core_topo_list
    {
        size_t items_reserved;
        size_t items_present;
        procmetrix_core_topo_key_t *data;
    } procmetrix_core_topo_list_t;

    //! Initializes the list of core topology entries
    //! @private
    PROCMETRIX_API void procmetrix_core_topo_list_init(procmetrix_core_topo_list_t *list);

    //! Frees the data in the list
    //! @private
    PROCMETRIX_API void procmetrix_core_topo_list_free(procmetrix_core_topo_list_t *list);

    //! Adds an item to the topology list
    //! @private
    PROCMETRIX_API void procmetrix_core_topo_list_add(procmetrix_core_topo_list_t *list, size_t package_id, size_t core_id);

    //! Compares the order of 2 topo keys
    //! @private
    PROCMETRIX_API int procmetrix_core_topo_key_compare(const procmetrix_core_topo_key_t *a, const procmetrix_core_topo_key_t *b);

    //! Sorts the list of core topo keys
    //! @private
    PROCMETRIX_API void procmetrix_core_topo_list_sort(procmetrix_core_topo_list_t *list);

    //! Counts the number of unique entries in the list
    //! @details Expects the list to be previously sorted by #procmetrix_core_topo_list_sort
    PROCMETRIX_API size_t procmetrix_core_topo_list_count_unique(const procmetrix_core_topo_list_t *list);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _PROCMETRIX_LINUX_INTERNAL_CORE_TOPOLOGY_H_
