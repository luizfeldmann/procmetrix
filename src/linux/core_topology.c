// Lib
#include "internal/linux/core_topology.h"

// STD
#include <limits.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// Impl

void procmetrix_core_topo_list_init(procmetrix_core_topo_list_t *list)
{
    // Sanity
    if (NULL == list)
        return;

    // Reserve initial capacity
    enum
    {
        K_INITIAL_CAPACITY = 128
    };

    list->items_reserved = K_INITIAL_CAPACITY;
    list->data = (procmetrix_core_topo_key_t *)malloc(list->items_reserved * sizeof(procmetrix_core_topo_key_t));

    // No actual items yet
    list->items_present = 0;
}

void procmetrix_core_topo_list_free(procmetrix_core_topo_list_t *list)
{
    if (NULL == list)
        return;

    free(list->data);
    memset(list, 0, sizeof(procmetrix_core_topo_list_t));
}

procmetrix_error_t procmetrix_core_topo_list_add(procmetrix_core_topo_list_t *list, size_t package_id, size_t core_id)
{
    if (NULL == list || NULL == list->data || 0 == list->items_reserved)
        return PROCMETRIX_ERROR_INVALID_ARGUMENT;

    // Check if list needs growing
    if (list->items_present >= list->items_reserved)
    {
        if (list->items_reserved > SIZE_MAX / sizeof(procmetrix_core_topo_key_t) / 2)
            return PROCMETRIX_ERROR_OUT_OF_MEMORY;

        // Increase capacity geometrically
        size_t new_capacity = list->items_reserved * 2;

        procmetrix_core_topo_key_t* new_data = (procmetrix_core_topo_key_t *)realloc(
            list->data, new_capacity * sizeof(procmetrix_core_topo_key_t));

        if (NULL == new_data)
            return PROCMETRIX_ERROR_OUT_OF_MEMORY;

        list->items_reserved = new_capacity;
        list->data = new_data;
    }

    // Add item
    list->data[list->items_present].package_id = package_id;
    list->data[list->items_present].core_id = core_id;
    ++list->items_present;

    return PROCMETRIX_ERROR_NONE;
}

int procmetrix_core_topo_key_compare(const procmetrix_core_topo_key_t *lhs, const procmetrix_core_topo_key_t *rhs)
{
    // Trivial case
    if (lhs == rhs)
        return 0;

    // First order by package id
    if (lhs->package_id < rhs->package_id)
        return -1;
    if (lhs->package_id > rhs->package_id)
        return 1;

    // Then order by core id
    if (lhs->core_id < rhs->core_id)
        return -1;
    if (lhs->core_id > rhs->core_id)
        return 1;

    // Equal
    return 0;
}

static int procmetrix_core_topo_key_compare_qsort(const void *lhs, const void *rhs)
{
    return procmetrix_core_topo_key_compare(
        (const procmetrix_core_topo_key_t *)lhs,
        (const procmetrix_core_topo_key_t *)rhs);
}

void procmetrix_core_topo_list_sort(procmetrix_core_topo_list_t *list)
{
    if (NULL == list)
        return;

    qsort(list->data, list->items_present,
          sizeof(procmetrix_core_topo_key_t), procmetrix_core_topo_key_compare_qsort);
}

size_t procmetrix_core_topo_list_count_unique(const procmetrix_core_topo_list_t *list)
{
    if (NULL == list)
        return 0;

    size_t unique_items = 0;

    for (size_t i = 0; i < list->items_present; ++i)
    {
        if (i == 0 || procmetrix_core_topo_key_compare(&list->data[i], &list->data[i - 1]) != 0)
            ++unique_items;
    }

    return unique_items;
}
