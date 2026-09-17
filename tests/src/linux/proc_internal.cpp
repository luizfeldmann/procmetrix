// Private impl
#include <internal/linux/proc_linux_internal.h>

// Testing
#include <gtest/gtest.h>

// Test cases

/** Parent PID */

TEST(procmetrix_impl_linux_proc_pid_stat_read_ppid, null_args)
{
    EXPECT_EQ(
        procmetrix_impl_linux_proc_pid_stat_read_ppid(nullptr, nullptr),
        PROCMETRIX_ERROR_INVALID_ARGUMENT);
}

TEST(procmetrix_impl_linux_proc_pid_stat_read_ppid, parent)
{
    procmetrix_pid_t parent_pid = 0;
    const char* stat_data = "pid (comm) S 1234";

    EXPECT_EQ(
        procmetrix_impl_linux_proc_pid_stat_read_ppid(stat_data, &parent_pid),
        PROCMETRIX_ERROR_NONE);

    EXPECT_EQ(parent_pid, 1234);
}
