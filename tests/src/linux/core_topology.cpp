// Private impl
#include <internal/linux/core_topology.h>

// Testing
#include <gtest/gtest.h>

// TEST CASES

TEST(procmetrix_core_topo_list_init, null_args)
{
    // No crash
    procmetrix_core_topo_list_init(nullptr);
}

TEST(procmetrix_core_topo_list_init, valid)
{
    procmetrix_core_topo_list_t list { 0 };
    procmetrix_core_topo_list_init(&list);

    // Init with zero items
    EXPECT_EQ(list.items_present, 0);

    // Starting reserved capacity
    EXPECT_GT(list.items_reserved, 0);

    // Staring buffer is valid
    EXPECT_NE(list.data, nullptr);

    // Cleanup
    procmetrix_core_topo_list_free(&list);
}

TEST(procmetrix_core_topo_list_free, null_args)
{
    // No crash
    procmetrix_core_topo_list_free(nullptr);
}

TEST(procmetrix_core_topo_list_free, valid)
{
    procmetrix_core_topo_list_t list { 0 };
    procmetrix_core_topo_list_init(&list);
    procmetrix_core_topo_list_free(&list);

    // All fields have been cleared
    EXPECT_EQ(list.items_present, 0);
    EXPECT_EQ(list.items_reserved, 0);
    EXPECT_EQ(list.data, nullptr);
}

TEST(procmetrix_core_topo_list_add, null_args)
{
    // No crash
    procmetrix_core_topo_list_add(nullptr, 0, 0);
}

TEST(procmetrix_core_topo_list_add, grow)
{
    // Initialize
    procmetrix_core_topo_list_t list { 0 };
    procmetrix_core_topo_list_init(&list);

    // Initial capacity
    size_t prev_capacity = list.items_reserved;

    // Fill items beyond initial capacity
    for (size_t i = 0; i < prev_capacity + 1; ++i)
        procmetrix_core_topo_list_add(&list, 0, i);

    // New capacity has grown
    EXPECT_GT(list.items_reserved, prev_capacity);

    // Item was succesfully inserted
    EXPECT_EQ(list.items_present, prev_capacity + 1);

    // Cleanup
    procmetrix_core_topo_list_free(&list);
}

TEST(procmetrix_core_topo_key_compare, order)
{
    procmetrix_core_topo_key_t a { 0, 0 };
    procmetrix_core_topo_key_t b { 0, 1 };
    procmetrix_core_topo_key_t c { 1, 0 };

    // Same items compare equal
    EXPECT_EQ(procmetrix_core_topo_key_compare(&a, &a), 0);

    // Order by core_id
    EXPECT_EQ(procmetrix_core_topo_key_compare(&a, &b), -1);
    EXPECT_EQ(procmetrix_core_topo_key_compare(&b, &a),  1);

    // Order by package_id
    EXPECT_EQ(procmetrix_core_topo_key_compare(&a, &c), -1);
    EXPECT_EQ(procmetrix_core_topo_key_compare(&b, &c), -1);
}

TEST(procmetrix_core_topo_list_sort, null_args)
{
    // No crash
    procmetrix_core_topo_list_sort(nullptr);
}

TEST(procmetrix_core_topo_list_count_unique, null_args)
{
    // No crash
    procmetrix_core_topo_list_count_unique(nullptr);
}

TEST(procmetrix_core_topo_list_count_unique, unique)
{
    procmetrix_core_topo_list_t list { 0 };
    procmetrix_core_topo_list_init(&list);

    // Add a repeated item but out of order
    procmetrix_core_topo_list_add(&list, 0, 1);
    procmetrix_core_topo_list_add(&list, 1, 0);
    procmetrix_core_topo_list_add(&list, 0, 1);

    // Sort and count unique
    procmetrix_core_topo_list_sort(&list);

    EXPECT_EQ(procmetrix_core_topo_list_count_unique(&list), 2);

    // Cleanup
    procmetrix_core_topo_list_free(&list);
}
