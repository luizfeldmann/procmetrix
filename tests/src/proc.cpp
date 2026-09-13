// Local lib
#include <procmetrix/proc.h>

// Testing
#include <gtest/gtest.h>

// STD
#include <algorithm>

// Test cases

TEST(procmetrix_pid_exists, own_pid)
{
    procmetrix_pid_t own_pid = procmetrix_get_pid();

    ASSERT_NE(own_pid, 0);
    EXPECT_TRUE(procmetrix_pid_exists(own_pid));
}

TEST(procmetrix_list_pids, null_args)
{
    size_t count = 0;
    procmetrix_pid_t* list = nullptr;

    // Null list
    EXPECT_EQ(
        procmetrix_list_pids(nullptr, &count),
            PROCMETRIX_ERROR_INVALID_ARGUMENT);
    
    // Null count
    EXPECT_EQ(
        procmetrix_list_pids(&list, nullptr),
            PROCMETRIX_ERROR_INVALID_ARGUMENT);
}

TEST(procmetrix_list_pids, contains_own_pid)
{
    size_t count = 0;
    procmetrix_pid_t* list = nullptr;

    // Get the list of all PIDs
    EXPECT_EQ(
        procmetrix_list_pids(&list, &count),
            PROCMETRIX_ERROR_NONE);
    
    // Has a valid, non-empty list
    EXPECT_NE(list, nullptr);
    EXPECT_NE(count, 0);

    // My own PID is contained in the list
    procmetrix_pid_t own_pid = procmetrix_get_pid();

    auto itend = std::next(list, count);
    auto itfind = std::find(list, itend, own_pid);

    EXPECT_NE(itfind, itend);

    // Cleanup
    procmetrix_free_pids(&list);
}