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

/** Memory info */

TEST(procmetrix_impl_linux_proc_read_statm, null_args)
{
    // Null input
    procmetrix_proc_memory_info_t memory_info { 0 };
    EXPECT_EQ(
        procmetrix_impl_linux_proc_read_statm(nullptr, &memory_info),
        PROCMETRIX_ERROR_INVALID_ARGUMENT);

    // Null output
    const char buf[] { "1 2 3 4 5 6 7" };
    EXPECT_EQ(
        procmetrix_impl_linux_proc_read_statm(buf, nullptr),
        PROCMETRIX_ERROR_INVALID_ARGUMENT);
}

TEST(procmetrix_impl_linux_proc_read_statm, invalid)
{
    // Only 6 of the required 7 fields
    const char buf[] { "1 2 3 4 5 6" };
    procmetrix_proc_memory_info_t memory_info { 0 };

    EXPECT_EQ(
        procmetrix_impl_linux_proc_read_statm(buf, &memory_info),
        PROCMETRIX_ERROR_MALFORMED);
}

TEST(procmetrix_impl_linux_proc_read_statm, valid)
{
    const char buf[] { "4062 1899 1572 1208 0 139 0" };
    procmetrix_proc_memory_info_t memory_info { 0 };

    EXPECT_EQ(
        procmetrix_impl_linux_proc_read_statm(buf, &memory_info),
        PROCMETRIX_ERROR_NONE);

    EXPECT_EQ(memory_info.vms, 4096 * 4062);
    EXPECT_EQ(memory_info.rss, 4096 * 1899);
    EXPECT_EQ(memory_info.shared, 4096 * 1572);
    EXPECT_EQ(memory_info.text, 4096 * 1208);
    EXPECT_EQ(memory_info.lib, 4096 * 0);
    EXPECT_EQ(memory_info.data, 4096 * 139);
    EXPECT_EQ(memory_info.dirty, 4096 * 0);
}

/** CPU times */

TEST(procmetrix_impl_linux_proc_pid_stat_read_cpu_times, null_args)
{
    // Null inputs
    procmetrix_proc_cpu_times_t cpu_times;
    EXPECT_EQ(
        procmetrix_impl_linux_proc_pid_stat_read_cpu_times(nullptr, &cpu_times),
        PROCMETRIX_ERROR_INVALID_ARGUMENT);

    // Null outputs
    const char buf[] { "1 (systemd) S" };
    EXPECT_EQ(
        procmetrix_impl_linux_proc_pid_stat_read_cpu_times(buf, nullptr),
        PROCMETRIX_ERROR_INVALID_ARGUMENT);
}

TEST(procmetrix_impl_linux_proc_pid_stat_read_cpu_times, invalid)
{
    // Missing fields
    const char buf[] { "1 (systemd) S" };
    procmetrix_proc_cpu_times_t cpu_times;

    EXPECT_EQ(
        procmetrix_impl_linux_proc_pid_stat_read_cpu_times(buf, &cpu_times),
        PROCMETRIX_ERROR_MALFORMED);
}

TEST(procmetrix_impl_linux_proc_pid_stat_read_cpu_times, valid)
{
    const char buf[] {
        "1 (systemd) S 1 2 3 4 5 6 7 8 9 10 1100 1200 1300 1400"
    };
    procmetrix_proc_cpu_times_t cpu_times;

    EXPECT_EQ(
        procmetrix_impl_linux_proc_pid_stat_read_cpu_times(buf, &cpu_times),
        PROCMETRIX_ERROR_NONE);

    EXPECT_DOUBLE_EQ(cpu_times.user, 11.0);
    EXPECT_DOUBLE_EQ(cpu_times.system, 12.0);
    EXPECT_DOUBLE_EQ(cpu_times.children_user, 13.0);
    EXPECT_DOUBLE_EQ(cpu_times.children_system, 14.0);
}
