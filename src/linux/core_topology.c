// Lib
#include "internal/linux/core_topology.h"

// STD
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
        kInitialCapacity = 128
    };

    list->items_reserved = kInitialCapacity;
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

void procmetrix_core_topo_list_add(procmetrix_core_topo_list_t *list, size_t package_id, size_t core_id)
{
    if (NULL == list)
        return;

    // Check if list needs growing
    if (list->items_present >= list->items_reserved)
    {
        list->items_reserved *= 2; // Increase capacity geometrically
        list->data = (procmetrix_core_topo_key_t *)realloc(
            list->data, list->items_reserved * sizeof(procmetrix_core_topo_key_t));
    }

    // Add item
    list->data[list->items_present].package_id = package_id;
    list->data[list->items_present].core_id = core_id;
    ++list->items_present;
}

int procmetrix_core_topo_key_compare(const procmetrix_core_topo_key_t *a, const procmetrix_core_topo_key_t *b)
{
    // Trivial case
    if (a == b)
        return 0;

    // First order by package id
    if (a->package_id < b->package_id)
        return -1;
    if (a->package_id > b->package_id)
        return 1;

    // Then order by core id
    if (a->core_id < b->core_id)
        return -1;
    if (a->core_id > b->core_id)
        return 1;

    // Equal
    return 0;
}

static int procmetrix_core_topo_key_compare_qsort(const void *a, const void *b)
{
    return procmetrix_core_topo_key_compare(
        (const procmetrix_core_topo_key_t *)a,
        (const procmetrix_core_topo_key_t *)b);
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
